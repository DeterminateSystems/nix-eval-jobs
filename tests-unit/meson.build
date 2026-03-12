nix_util_test_support_dep = dependency('nix-util-test-support')

test_exe = executable(
  'nix-eval-jobs-unit-tests',
  'json.cc',
  dependencies: [nix_eval_jobs_dep, gtest_dep, nix_util_test_support_dep],
)

test(
  'nix-eval-jobs-unit-tests',
  test_exe,
  protocol: 'gtest',
  env: {
    '_NIX_TEST_UNIT_DATA': meson.current_source_dir() / 'data',
  },
)
