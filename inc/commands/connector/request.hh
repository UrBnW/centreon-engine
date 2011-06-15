/*
** Copyright 2011      Merethis
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

#ifndef CCE_COMMANDS_CONNECTOR_REQUEST_HH
# define CCE_COMMANDS_CONNECTOR_REQUEST_HH

# include <QByteArray>

namespace                            com {
  namespace                          centreon {
    namespace                        engine {
      namespace                      commands {
	namespace                    connector {
	  class                      request {
	  public:
	    enum                     e_type {
	      version_q = 0,
	      version_r,
	      execute_q,
	      execute_r,
	      quit_q,
	      quit_r
	    };

	                             request(e_type id);
	                             request(request const& right);
	    virtual                  ~request() throw();

	    request&                 operator=(request const& right);

	    static QByteArray const& cmd_ending() throw();

	    virtual e_type           get_id() const throw();

	    virtual request*         clone() const = 0;
	    virtual QByteArray       build() = 0;
	    virtual void             restore(QByteArray const& data) = 0;

	  protected:
	    e_type                   _id;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_REQUEST_HH