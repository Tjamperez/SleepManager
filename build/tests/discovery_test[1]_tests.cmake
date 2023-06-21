add_test( DiscoveryTest.SendDiscoveryMessageTest /home/thales/projects/SleepManager/build/tests/discovery_test [==[--gtest_filter=DiscoveryTest.SendDiscoveryMessageTest]==] --gtest_also_run_disabled_tests)
set_tests_properties( DiscoveryTest.SendDiscoveryMessageTest PROPERTIES WORKING_DIRECTORY /home/thales/projects/SleepManager/build/tests SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set( discovery_test_TESTS DiscoveryTest.SendDiscoveryMessageTest)
