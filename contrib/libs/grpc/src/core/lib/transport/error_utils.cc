/*
 *
 * Copyright 2016 gRPC authors.
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

#include "src/core/lib/transport/error_utils.h"

#include <stdint.h>

#include <vector>

#include "y_absl/strings/string_view.h"

#include <grpc/support/string_util.h>

#include "src/core/lib/gprpp/status_helper.h"
#include "src/core/lib/transport/status_conversion.h"

static grpc_error_handle recursively_find_error_with_field(
    grpc_error_handle error, grpc_error_ints which) {
  intptr_t unused;
  // If the error itself has a status code, return it.
  if (grpc_error_get_int(error, which, &unused)) {
    return error;
  }
  std::vector<y_absl::Status> children = grpc_core::StatusGetChildren(error);
  for (const y_absl::Status& child : children) {
    grpc_error_handle result = recursively_find_error_with_field(child, which);
    if (!GRPC_ERROR_IS_NONE(result)) return result;
  }
  return GRPC_ERROR_NONE;
}

void grpc_error_get_status(grpc_error_handle error,
                           grpc_core::Timestamp deadline,
                           grpc_status_code* code, TString* message,
                           grpc_http2_error_code* http_error,
                           const char** error_string) {
  // Fast path: We expect no error.
  if (GPR_LIKELY(GRPC_ERROR_IS_NONE(error))) {
    if (code != nullptr) *code = GRPC_STATUS_OK;
    if (message != nullptr) {
      // Normally, we call grpc_error_get_str(
      //   error, GRPC_ERROR_STR_GRPC_MESSAGE, message).
      // We can fastpath since we know that:
      // 1) Error is null
      // 2) which == GRPC_ERROR_STR_GRPC_MESSAGE
      // 3) The resulting message is statically known.
      // 4) Said resulting message is "".
      // This means 3 movs, instead of 10s of instructions and a strlen.
      *message = "";
    }
    if (http_error != nullptr) {
      *http_error = GRPC_HTTP2_NO_ERROR;
    }
    return;
  }

  // Start with the parent error and recurse through the tree of children
  // until we find the first one that has a status code.
  grpc_error_handle found_error =
      recursively_find_error_with_field(error, GRPC_ERROR_INT_GRPC_STATUS);
  if (GRPC_ERROR_IS_NONE(found_error)) {
    /// If no grpc-status exists, retry through the tree to find a http2 error
    /// code
    found_error =
        recursively_find_error_with_field(error, GRPC_ERROR_INT_HTTP2_ERROR);
  }

  // If we found an error with a status code above, use that; otherwise,
  // fall back to using the parent error.
  if (GRPC_ERROR_IS_NONE(found_error)) found_error = error;

  grpc_status_code status = GRPC_STATUS_UNKNOWN;
  intptr_t integer;
  if (grpc_error_get_int(found_error, GRPC_ERROR_INT_GRPC_STATUS, &integer)) {
    status = static_cast<grpc_status_code>(integer);
  } else if (grpc_error_get_int(found_error, GRPC_ERROR_INT_HTTP2_ERROR,
                                &integer)) {
    status = grpc_http2_error_to_grpc_status(
        static_cast<grpc_http2_error_code>(integer), deadline);
  } else {
    status = static_cast<grpc_status_code>(found_error.code());
  }
  if (code != nullptr) *code = status;

  if (error_string != nullptr && status != GRPC_STATUS_OK) {
    *error_string = gpr_strdup(grpc_error_std_string(error).c_str());
  }

  if (http_error != nullptr) {
    if (grpc_error_get_int(found_error, GRPC_ERROR_INT_HTTP2_ERROR, &integer)) {
      *http_error = static_cast<grpc_http2_error_code>(integer);
    } else if (grpc_error_get_int(found_error, GRPC_ERROR_INT_GRPC_STATUS,
                                  &integer)) {
      *http_error =
          grpc_status_to_http2_error(static_cast<grpc_status_code>(integer));
    } else {
      *http_error = GRPC_ERROR_IS_NONE(found_error) ? GRPC_HTTP2_NO_ERROR
                                                    : GRPC_HTTP2_INTERNAL_ERROR;
    }
  }

  // If the error has a status message, use it.  Otherwise, fall back to
  // the error description.
  if (message != nullptr) {
    if (!grpc_error_get_str(found_error, GRPC_ERROR_STR_GRPC_MESSAGE,
                            message)) {
      if (!grpc_error_get_str(found_error, GRPC_ERROR_STR_DESCRIPTION,
                              message)) {
        *message = grpc_error_std_string(error);
      }
    }
  }
}

y_absl::Status grpc_error_to_absl_status(grpc_error_handle error) {
  grpc_status_code status;
  // TODO(yashykt): This should be updated once we decide on how to use the
  // y_absl::Status payload to capture all the contents of grpc_error.
  TString message;
  grpc_error_get_status(error, grpc_core::Timestamp::InfFuture(), &status,
                        &message, nullptr /* http_error */,
                        nullptr /* error_string */);
  return y_absl::Status(static_cast<y_absl::StatusCode>(status), message);
}

grpc_error_handle absl_status_to_grpc_error(y_absl::Status status) {
  // Special error checks
  if (status.ok()) {
    return GRPC_ERROR_NONE;
  }
  return grpc_error_set_int(
      GRPC_ERROR_CREATE_FROM_STRING_VIEW(status.message()),
      GRPC_ERROR_INT_GRPC_STATUS, static_cast<grpc_status_code>(status.code()));
}

bool grpc_error_has_clear_grpc_status(grpc_error_handle error) {
  intptr_t unused;
  if (grpc_error_get_int(error, GRPC_ERROR_INT_GRPC_STATUS, &unused)) {
    return true;
  }
  std::vector<y_absl::Status> children = grpc_core::StatusGetChildren(error);
  for (const y_absl::Status& child : children) {
    if (grpc_error_has_clear_grpc_status(child)) {
      return true;
    }
  }
  return false;
}
