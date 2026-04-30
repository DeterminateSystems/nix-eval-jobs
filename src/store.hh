#pragma once
///@file

#include <nix/store/store-api.hh>
#include <nix/store/store-open.hh>
#include <nix/store/store-reference.hh>
#include <nix/util/ref.hh>
#include <optional>

namespace nix_eval_jobs {

// Helper to open a store from an optional store reference
inline auto
openStore(std::optional<nix::StoreReference> storeRef = std::nullopt)
    -> nix::ref<nix::Store> {
    return storeRef ? nix::openStore(std::move(*storeRef)) : nix::openStore();
}

} // namespace nix_eval_jobs
