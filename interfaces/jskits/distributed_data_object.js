const distributeddataobject = requireInternal("data.distributedDataObject");

function changeSession(obj, sessionId = "") {
    console.info("hanlu start changeSession");
    if (obj == null) {
        console.error("object is null");
        return obj;
    }

    if (sessionId.length == 0) {
        let object = obj;
        if (obj.sessionId != null && obj.sessionId != undefined && obj.sessionId.length > 0) {
            // disconnect,delete object
            distributeddataobject.destroyObjectSync(obj.sessionId);
            /* Object.keys(obj).forEach(key => {
                 Object.defineProperty(object, key, {
                     value:obj[key],
                     configurable:true,
                     writable:true,
                     enumerable:true,
                     get: function () {
                         return object[key];
                     },
                     set: function (newValue) {
                         object[key] = newValue;
                     }
                 })
                 Object.defineProperty(obj, key, {
                     value:obj[key],
                     configurable:true,
                     writable:true,
                     enumerable:true,
                     get: function () {
                         return obj[key];
                     },
                     set: function (newValue) {
                         obj[key] = newValue;
                     }
                 })

             });*/
        }
        return object;
    } else {
        console.info("hanlu start createObjectSync");
        /*let object = distributeddataobject.createObjectSync(sessionId);
        Object.keys(obj).forEach(key => {
            Object.defineProperty(object, key, {
                value:undefined,
                configurable:true,
                writable:true,
                enumerable:true,
                get: function () {
                    return object.get(key);
                },
                set: function (newValue) {
                    object.put(key, newValue);
                }
            });
            Object.defineProperty(object, "sessionId", {
                value:sessionId,
                configurable:true,
            })
        });*/
        return object;
    }
}

function on(type, obj, callback) {
    console.info("hanlu start on");
    if (obj.sessionId != null && obj.sessionId != undefined && obj.sessionId.length > 0) {
        distributeddataobject.on(type, obj, callback);
    }
}

function off(type, obj, callback = undefined) {
    console.info("hanlu start off");
    if (obj.sessionId != null && obj.sessionId != undefined && obj.sessionId.length > 0) {
        distributeddataobject.off(type, obj, callback);
    }
}
export default {
    joinSession: changeSession,
    leaveSession: changeSession,
    on: on,
    off: off,
    createObjectSync: distributeddataobject.createObjectSync,
    destroyObjectSync: distributeddataobject.destroyObjectSync,
}