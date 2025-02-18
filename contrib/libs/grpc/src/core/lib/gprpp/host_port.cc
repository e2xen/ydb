/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpc/support/port_platform.h>

#include "src/core/lib/gprpp/host_port.h"

#include <stddef.h>

#include "y_absl/strings/str_format.h"
#include "y_absl/strings/string_view.h"

#include <grpc/support/log.h>

namespace grpc_core {

TString JoinHostPort(y_absl::string_view host, int port) {
  if (!host.empty() && host[0] != '[' && host.rfind(':') != host.npos) {
    // IPv6 literals must be enclosed in brackets.
    return y_absl::StrFormat("[%s]:%d", host, port);
  }
  // Ordinary non-bracketed host:port.
  return y_absl::StrFormat("%s:%d", host, port);
}

namespace {
bool DoSplitHostPort(y_absl::string_view name, y_absl::string_view* host,
                     y_absl::string_view* port, bool* has_port) {
  *has_port = false;
  if (!name.empty() && name[0] == '[') {
    /* Parse a bracketed host, typically an IPv6 literal. */
    const size_t rbracket = name.find(']', 1);
    if (rbracket == y_absl::string_view::npos) {
      /* Unmatched [ */
      return false;
    }
    if (rbracket == name.size() - 1) {
      /* ]<end> */
      *port = y_absl::string_view();
    } else if (name[rbracket + 1] == ':') {
      /* ]:<port?> */
      *port = name.substr(rbracket + 2, name.size() - rbracket - 2);
      *has_port = true;
    } else {
      /* ]<invalid> */
      return false;
    }
    *host = name.substr(1, rbracket - 1);
    if (host->find(':') == y_absl::string_view::npos) {
      /* Require all bracketed hosts to contain a colon, because a hostname or
         IPv4 address should never use brackets. */
      *host = y_absl::string_view();
      return false;
    }
  } else {
    size_t colon = name.find(':');
    if (colon != y_absl::string_view::npos &&
        name.find(':', colon + 1) == y_absl::string_view::npos) {
      /* Exactly 1 colon.  Split into host:port. */
      *host = name.substr(0, colon);
      *port = name.substr(colon + 1, name.size() - colon - 1);
      *has_port = true;
    } else {
      /* 0 or 2+ colons.  Bare hostname or IPv6 litearal. */
      *host = name;
      *port = y_absl::string_view();
    }
  }
  return true;
}
}  // namespace

bool SplitHostPort(y_absl::string_view name, y_absl::string_view* host,
                   y_absl::string_view* port) {
  bool unused;
  return DoSplitHostPort(name, host, port, &unused);
}

bool SplitHostPort(y_absl::string_view name, TString* host,
                   TString* port) {
  GPR_DEBUG_ASSERT(host != nullptr && host->empty());
  GPR_DEBUG_ASSERT(port != nullptr && port->empty());
  y_absl::string_view host_view;
  y_absl::string_view port_view;
  bool has_port;
  const bool ret = DoSplitHostPort(name, &host_view, &port_view, &has_port);
  if (ret) {
    // We always set the host, but port is set only when DoSplitHostPort find a
    // port in the string, to remain backward compatible with the old
    // gpr_split_host_port API.
    *host = TString(host_view);
    if (has_port) {
      *port = TString(port_view);
    }
  }
  return ret;
}

}  // namespace grpc_core
