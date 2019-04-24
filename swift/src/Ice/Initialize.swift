//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

// Factories are registed once, when this file is loaded
private let initialized: Bool = {
    ICEUtil.registerFactories(exception: ExceptionFactory.self,
                              connectionInfo: ConnectionInfoFactory.self,
                              endpointInfo: EndpointInfoFactory.self,
                              adminFacet: AdminFacetFactory.self)
    return true
}()

public func initialize(args: StringSeq = [],
                       initData userInitData: InitializationData? = nil,
                       configFile: String? = nil) throws -> Communicator {
    // Ensure factories are initialized
    guard initialized else {
        fatalError("Unable to initialie Ice")
    }

    return try autoreleasepool {
        var initData = userInitData ?? InitializationData()

        //
        // Properties
        //
        if initData.properties == nil {
            let (props, _) = try Ice.createProperties()
            initData.properties = props
        }

        if configFile != nil {
            try initData.properties!.load(configFile!)
        }

        //
        // Logger
        //
        // precedence:
        // - initData.logger
        // - logger property
        // - C++ plugin loggers
        // - Swift logger
        //
        // If no user logger or property has been specified then use the Swift logger.
        // This logger may be overwritten by a logger plug-in during initialization
        if initData.logger == nil,
            initData.properties!.getProperty("Ice.LogFile").isEmpty,
            initData.properties!.getProperty("Ice.UseSyslog").isEmpty {
            initData.logger = LoggerI()
        }

        var loggerP: ICELoggerProtocol?
        if let l = initData.logger {
            loggerP = l as? LoggerI ?? LoggerWrapper(handle: l)
        }

        if let l = initData.logger {
            loggerP = l as? LoggerI ?? LoggerWrapper(handle: l)
        }

        let propsHandle = (initData.properties as! PropertiesI)._handle
        let handle = try ICEUtil.initialize(args,
                                            properties: propsHandle,
                                            logger: loggerP)

        //
        // Update initData.properties reference to point to the properties object
        // created by Ice::initialize.
        //
        initData.properties = PropertiesI(handle: handle.getProperties())

        //
        // Update initData.logger referecnce in case we are using a C++ logger (defined though a property) or
        //  a C++ logger plug-in installed a new logger
        //
        if let objcLogger = handle.getLogger() as? ICELogger {
            initData.logger = objcLogger.fromLocalObject(to: ObjcLoggerWrapper.self) { ObjcLoggerWrapper(handle: objcLogger) }
        }

        precondition(initData.logger != nil && initData.properties != nil)

        return CommunicatorI(handle: handle, initData: initData)
    }
}

public func createProperties(args: StringSeq? = nil, defaults: Properties? = nil) throws -> (Properties, StringSeq) {
    return try autoreleasepool {
        var remArgs: NSArray?
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?._handle,
                                                            remArgs: &remArgs)
        // swiftlint:disable force_cast
        return (PropertiesI(handle: propertiesHandle), remArgs as! StringSeq)
    }
}

public let intVersion: Int = 30702
public let stringVersion: String = "3.7.2"

public var currentEncoding: EncodingVersion {
    var encoding = EncodingVersion()
    ICEUtil.currentEncoding(major: &encoding.major, minor: &encoding.minor)
    return encoding
}

public func stringToIdentity(_ string: String) throws -> Identity {
    return try autoreleasepool {
        var name = NSString()
        var category = NSString()
        try ICEUtil.stringToIdentity(str: string, name: &name, category: &category)
        return Identity(name: name as String, category: category as String)
    }
}

public func identityToString(id: Identity, mode: ToStringMode = ToStringMode.Unicode) -> String {
    return ICEUtil.identityToString(name: id.name, category: id.category, mode: mode.rawValue)
}

public func encodingVersionToString(_ encoding: EncodingVersion) -> String {
    return ICEUtil.encodingVersionToString(major: encoding.major, minor: encoding.minor)
}

public let Encoding_1_0 = EncodingVersion(major: 1, minor: 0)
public let Encoding_1_1 = EncodingVersion(major: 1, minor: 1)
