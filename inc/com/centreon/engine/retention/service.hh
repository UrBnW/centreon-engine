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

#ifndef CCE_RETENTION_SERVICE_HH
#  define CCE_RETENTION_SERVICE_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/retention/object.hh"

// forward declaration.
struct service_struct;

CCE_BEGIN()

namespace           retention {
  class             service
    : public object {
  public:
                    service(service_struct* obj = NULL);
                    ~service() throw ();
    bool            set(
                      std::string const& key,
                      std::string const& value);

  private:
    void            _finished() throw ();
    bool            _modified_attributes(
                      std::string const& key,
                      std::string const& value);
    bool            _retain_nonstatus_information(
                      std::string const& key,
                      std::string const& value);
    bool            _retain_status_information(
                      std::string const& key,
                      std::string const& value);

    std::string     _host_name;
    service_struct* _obj;
    std::string     _service_description;
    bool            _was_flapping;
  };
}

CCE_END()

#endif // !CCE_RETENTION_SERVICE_HH
