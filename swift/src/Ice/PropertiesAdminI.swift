//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class PropertiesAdminI: LocalObject<ICEPropertiesAdmin>, PropertiesAdmin, NativePropertiesAdmin {
    let communicator: Communicator

    init(communicator: Communicator, handle: ICEPropertiesAdmin) {
        self.communicator = communicator
        super.init(handle: handle)
    }

    func getProperty(key: Swift.String, current _: Current) throws -> Swift.String {
        return try autoreleasepool {
            try _handle.getProperty(key)
        }
    }

    func getPropertiesForPrefix(prefix: Swift.String, current _: Current) throws -> PropertyDict {
        return try autoreleasepool {
            try _handle.getPropertiesForPrefix(prefix)
        }
    }

    func setProperties(newProperties: PropertyDict, current _: Current) throws {
        try autoreleasepool {
            try _handle.setProperties(newProperties)
        }
    }

    func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveCallback {
        return _handle.addUpdateCallback { (props: PropertyDict) in
            cb(props)
        }
    }
}
