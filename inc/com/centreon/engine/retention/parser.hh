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

#ifndef CCE_RETENTION_PARSER_HH
#  define CCE_RETENTION_PARSER_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace              retention {
  class                state;

  class                parser {
  public:
                       parser();
                       ~parser() throw ();
    void               parse(std::string const& path, state& retention);
  };
}

CCE_END()

#endif // !CCE_RETENTION_PARSER_HH