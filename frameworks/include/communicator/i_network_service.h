/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 */

#ifndef I_NETWORK_SERVICE_H
#define I_NETWORK_SERVICE_H

namespace OHOS::ObjectStore {
namespace {
const std::u16string DATA_SERVICE_INTERFACE_TOKEN = u"objectstore.data.service";
}
class INetworkService : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(DATA_SERVICE_INTERFACE_TOKEN);
    enum TRANSACTION_CODE {
        CODE_BASE = 0,
        SEND_PROXY_TO_REMOTE = CODE_BASE + 1,
        SEND_DATA = CODE_BASE + 2,
    };
    INetworkService() = default;
    virtual ~INetworkService() = default;
    virtual int32_t SendProxyToRemote(const std::string &deviceId, const std::string &bundleName,
                                      const std::string &serviceToken, const sptr<IRemoteObject> &remoteProxy) = 0;
    virtual int32_t SendData(const void *data, size_t len) = 0;
};
}  // namespace OHOS::ObjectStore
#endif