/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <exception>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run enable_servicegroup_svc_checks test.
 */
static int check_enable_servicegroup_svc_checks(int argc, char** argv) {
  (void)argc;
  (void)argv;

  service* svc = add_service(
      "name", "description", NULL, NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
      0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0);
  if (!svc)
    throw(engine_error() << "create service failed.");

  servicegroup* group = add_servicegroup("group", NULL, NULL, NULL, NULL);
  if (!group)
    throw(engine_error() << "create servicegroup failed.");

  servicesmember* member =
      add_service_to_servicegroup(group, "name", "description");
  if (!member)
    throw(engine_error() << "create servicemember failed.");
  member->service_ptr = svc;

  svc->checks_enabled = false;
  char const* cmd("[1317196300] ENABLE_SERVICEGROUP_SVC_CHECKS;group");
  process_external_command(cmd);

  if (!svc->checks_enabled)
    throw(engine_error() << "enable_servicegroup_svc_checks failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_enable_servicegroup_svc_checks);
  return (utest.run());
}
