/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#include <cstring>
#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include <time.h>
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include <com/centreon/process_manager.hh>
#include "com/centreon/engine/error.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/timezone_manager.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class HostFlappingNotification : public ::testing::Test {
 public:
  void SetUp() override {
    if (!config)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(
    timezone_manager::load();

    configuration::applier::contact ct_aply;
    configuration::contact ctct{valid_contact_config()};
    process_manager::load();
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::applier::host hst_aply;
    configuration::host hst;
    hst.parse("host_name", "test_host");
    hst.parse("address", "127.0.0.1");
    hst.parse("_HOST_ID", "12");
    hst.parse("contacts", "admin");
    hst_aply.add_object(hst);
    hst_aply.resolve_object(hst);
    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    process_manager::unload();
    timezone_manager::unload();
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

  configuration::contact valid_contact_config() const {
    // Add command.
    {
      configuration::command cmd;
      cmd.parse("command_name", "cmd");
      cmd.parse("command_line", "true");
      configuration::applier::command aplyr;
      aplyr.add_object(cmd);
    }
    // Add timeperiod.
    {
      configuration::timeperiod tperiod;
      tperiod.parse("timeperiod_name", "24x7");
      tperiod.parse("alias", "24x7");
      tperiod.parse("monday", "00:00-24:00");
      tperiod.parse("tuesday", "00:00-24:00");
      tperiod.parse("wednesday", "00:00-24:00");
      tperiod.parse("thursday", "00:00-24:00");
      tperiod.parse("friday", "00:00-24:00");
      tperiod.parse("saterday", "00:00-24:00");
      tperiod.parse("sunday", "00:00-24:00");
      configuration::applier::timeperiod aplyr;
      aplyr.add_object(tperiod);
    }
    // Valid contact configuration
    // (will generate 0 warnings or 0 errors).
    configuration::contact ctct;
    ctct.parse("contact_name", "admin");
    ctct.parse("host_notification_period", "24x7");
    ctct.parse("service_notification_period", "24x7");
    ctct.parse("host_notification_commands", "cmd");
    ctct.parse("service_notification_commands", "cmd");
    ctct.parse("host_notification_options", "r,f");
    ctct.parse("host_notifications_enabled", "1");
    return ctct;
  }

 protected:
  std::shared_ptr<engine::host> _host;
};

// Given a host UP
// When it is flapping
// Then it can throw a flappingstart notification followed by a recovery
// notification.
// When it is no more flapping
// Then it can throw a flappingstop notification followed by a recovery
// notification.
TEST_F(HostFlappingNotification, SimpleHostFlapping) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43000);
  // FIXME DBR: should not we find a better solution than fixing this each time?
  _host->set_last_hard_state_change(43000);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (size_t i = 0; i < tperiod->days.size(); ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->notification_period_ptr = tperiod.get();
  _host->set_is_flapping(true);
  testing::internal::CaptureStdout();
  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
  set_time(43500);
  _host->set_is_flapping(false);
  ASSERT_EQ(_host->notify(notifier::reason_flappingstop, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 2, _host->get_next_notification_id());

  ASSERT_EQ(_host->notify(notifier::reason_recovery, "", "",
                          notifier::notification_option_none),
            OK);

  std::string out{testing::internal::GetCapturedStdout()};
  size_t step1{out.find("HOST NOTIFICATION: admin;test_host;FLAPPINGSTART (UP);cmd;")};
  size_t step2{out.find("HOST NOTIFICATION: admin;test_host;FLAPPINGSTART (UP);cmd;")};
  ASSERT_NE(step1, std::string::npos);
  ASSERT_NE(step2, std::string::npos);
  ASSERT_LE(step1, step2);

  ASSERT_EQ(id + 3, _host->get_next_notification_id());
}

// Given a host UP
// When it is flapping
// Then it can throw a flappingstart notification followed by a recovery
// notification.
// When a second flappingstart notification is sent
// Then no notification is sent (because already sent).
TEST_F(HostFlappingNotification, SimpleHostFlappingStartTwoTimes) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43000);
  _host->set_notification_interval(2);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (uint32_t i = 0; i < tperiod->days.size(); ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->notification_period_ptr = tperiod.get();
  _host->set_is_flapping(true);
  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());

  set_time(43050);
  /* Notification already sent, no notification should be sent. */
  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());
}

// Given a host UP
// When it is flapping
// Then it can throw a flappingstart notification followed by a recovery
// notification.
// When a flappingstop notification is sent
// Then it is sent.
// When a second flappingstop notification is sent
// Then nothing is sent.
TEST_F(HostFlappingNotification, SimpleHostFlappingStopTwoTimes) {
  /* We are using a local time() function defined in tests/timeperiod/utils.cc.
   * If we call time(), it is not the glibc time() function that will be called.
   */
  set_time(43000);
  _host->set_notification_interval(2);
  std::unique_ptr<engine::timeperiod> tperiod{
      new engine::timeperiod("tperiod", "alias")};
  for (uint32_t i = 0; i < tperiod->days.size(); ++i)
    tperiod->days[i].push_back(std::make_shared<engine::timerange>(0, 86400));

  std::unique_ptr<engine::hostescalation> host_escalation{
      new engine::hostescalation("host_name", 0, 1, 1.0, "tperiod", 7)};

  ASSERT_TRUE(host_escalation);
  uint64_t id{_host->get_next_notification_id()};
  _host->notification_period_ptr = tperiod.get();
  _host->set_is_flapping(true);
  ASSERT_EQ(_host->notify(notifier::reason_flappingstart, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 1, _host->get_next_notification_id());

  set_time(43050);
  /* Flappingstop notification: sent. */
  ASSERT_EQ(_host->notify(notifier::reason_flappingstop, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 2, _host->get_next_notification_id());

  set_time(43100);
  /* Second flappingstop notification: not sent. */
  ASSERT_EQ(_host->notify(notifier::reason_flappingstop, "", "",
                          notifier::notification_option_none),
            OK);
  ASSERT_EQ(id + 2, _host->get_next_notification_id());
}