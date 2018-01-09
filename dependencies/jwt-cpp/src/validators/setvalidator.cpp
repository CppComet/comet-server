// Copyright (c) 2015 Erwin Jansen
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "jwt/setvalidator.h"
#include <sstream>
#include <string>
#include <vector>

SetValidator::SetValidator(const std::vector<MessageValidator *> &validators) {
    for (auto validator : validators) {
        validator_map_[validator->algorithm()] = validator;
    }
}

bool SetValidator::Verify(const json &jose, const uint8_t *header,
                          size_t num_header, const uint8_t *signature,
                          size_t num_signature) const {
    if (!jose.count("alg")) {
        return false;
    }

    auto alg = validator_map_.find(jose["alg"].get<std::string>());
    if (alg == validator_map_.end()) {
        return false;
    }

    MessageValidator *validator = alg->second;
    return validator->Verify(nullptr, header, num_header, signature,
                             num_signature);
}

bool SetValidator::Accepts(const json &jose) const {
    if (!jose.count("alg") || !jose["alg"].is_string()) return false;

    auto alg = validator_map_.find(jose["alg"].get<std::string>());
    if (alg == validator_map_.end()) {
        return false;
    }
    return alg->second->Accepts(jose);
}

std::string SetValidator::toJson() const {
    std::ostringstream msg;
    msg << "{ \"set\" : [ ";
    int idx = 0;
    for (const auto &validator : validator_map_) {
        if (idx++ > 0) {
            msg << ", ";
        }
        msg << validator.second->toJson();
    }
    msg << " ] }";
    return msg.str();
}
