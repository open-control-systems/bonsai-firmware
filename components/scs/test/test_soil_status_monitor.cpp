/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unity.h"

#include "ocs_core/noncopyable.h"
#include "ocs_test/test_clock.h"
#include "ocs_test/test_counter_storage.h"
#include "scs/soil_status_monitor.h"

namespace ocs {
namespace app {

namespace {

struct TestCounterHolder : public diagnostic::BasicCounterHolder,
                           public core::NonCopyable<> {
    unsigned count() const {
        return get_counters_().size();
    }
};

} // namespace

TEST_CASE("Soil status monitor: counters registration", "[soil_status_monitor], [scs]") {
    test::TestClock clock;
    test::TestCounterStorage storage;

    TestCounterHolder counter_holder;
    system::FanoutRebootHandler reboot_handler;

    SoilStatusMonitor monitor(clock, storage, reboot_handler, counter_holder);
    TEST_ASSERT_EQUAL(2, counter_holder.count());
}

} // namespace app
} // namespace ocs
