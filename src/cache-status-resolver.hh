#pragma once
///@file

#include <boost/asio/awaitable.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <nix/store/derivations.hh>
#include <nix/store/path.hh>
#include <optional>
#include <nix/store/store-api.hh>
#include <nix/util/types.hh>
#include <set>
#include <nix/util/ref.hh>
#include <thread>
#include <vector>

#include "drv.hh"
#include "response.hh"

/* Resolves the cache status (substituter narinfo lookups) of evaluated
   jobs with asio coroutines on a single io thread.

   Doing the lookup inline in the eval worker stalls CPU-bound
   evaluation behind HTTP round-trips, so the collector hands
   finished jobs over to this resolver instead. The probes use the
   store's asynchronous
   queryPathInfo, so one thread suffices to keep many lookups in
   flight. */
class CacheStatusResolver {
  public:
    /* Called with the job after its cache status has been filled in.
       Invoked from the io thread; the sink must do its own locking. */
    using Sink = std::function<void(Response)>;

    CacheStatusResolver(nix::ref<nix::Store> store, Sink sink);
    ~CacheStatusResolver();

    CacheStatusResolver(const CacheStatusResolver &) = delete;
    CacheStatusResolver(CacheStatusResolver &&) = delete;
    auto operator=(const CacheStatusResolver &)
        -> CacheStatusResolver & = delete;
    auto operator=(CacheStatusResolver &&) -> CacheStatusResolver & = delete;

    void push(Response response);

    /* Drain remaining work, stop the io thread and rethrow the first
       error raised by a job. */
    void finish();

  private:
    /* Per-job state of the slow-path input graph walk. */
    struct Traversal {
        Drv &drv;
        nix::StorePathSet visited;
        std::set<nix::StorePath> substitutePaths;
    };

    auto process(Response response) -> boost::asio::awaitable<void>;
    auto visitDrv(Traversal *traversal, nix::StorePath drvPath,
                  nix::StringSet wantedOutputs) -> boost::asio::awaitable<void>;
    auto missingOutputs(const nix::Derivation &derivation,
                        const nix::StringSet &wantedOutputs)
        -> std::optional<std::vector<nix::StorePath>>;
    auto substitutable(nix::StorePath path) -> boost::asio::awaitable<bool>;
    auto allSubstitutable(std::vector<nix::StorePath> paths)
        -> boost::asio::awaitable<bool>;
    void throwIfAborted() const;
    void onJobDone(const std::exception_ptr &error);
    void stopAndJoin();

    nix::ref<nix::Store> store;
    std::list<nix::ref<nix::Store>> substituters;
    Sink sink;

    boost::asio::io_context ctx;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        workGuard;
    std::thread ioThread;

    /* Concurrency is bounded by nix's own http-connections limit per
       substituter, so jobs are spawned as they arrive; there is no
       in-flight cap of our own. */
    std::mutex mutex;
    std::condition_variable idle;
    size_t inFlight = 0;
    bool closed = false;
    std::exception_ptr exc;
    /* Cooperative cancellation, checked by the coroutines between
       awaits. Atomic because the destructor sets it from another
       thread without holding the mutex. */
    std::atomic<bool> aborted = false;

    /* Jobs share most of their closure (e.g. stdenv), so remember
       per-path probe results across jobs. Only ever touched from the
       io thread. */
    std::map<nix::StorePath, bool> probeCache;
};
