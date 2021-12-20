/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
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
*/
import { AsyncCallback, Callback } from './basic';

/**
 * Provides interfaces to obtain and modify storage data.
 *
 * @name distributeddataobject
 * @since 8
 * @sysCap
 * @devices phone, tablet
 */
declare namespace distributedobjectstore {
    function joinSession(obj: object, sessionId: string): DistributedObject;
    function leaveSession(obj: DistributedObject): object;
    function on(type: 'change', object: DistributedObject, callback: Callback<ChangedDataObserver>): void;
    function off(type: 'change', object: DistributedObject, callback?: Callback<ChangedDataObserver>): void;
    function on(type: 'status', object: DistributedObject, callback: Callback<ObjectStatusObserver>): void;
    function off(type: 'status', object: DistributedObject, callback?: Callback<ObjectStatusObserver>): void;


    type ValueType = number | string | boolean | object;
    class DistributedObject {
        put(key: string, value: ValueType): void;
        get(key: string): ValueType;
    }

    interface ChangedDataObserver {
        sessionId: string;
        changeData: Array<string>;
    }

    interface ObjectStatusObserver {
        deviceId: string;
        sessionId: string;
        status: "connected"|"disconnected";
    }
}

export default distributedobjectstore;