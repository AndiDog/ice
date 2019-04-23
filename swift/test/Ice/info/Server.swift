//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Dispatch

class Server: TestHelperI {
    public override func run(args: [String]) throws {

        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }

        communicator.getProperties().setProperty(
          key: "TestAdapter.Endpoints",
          value: "\(getTestEndpoint(num: 0)):\(getTestEndpoint(num: 0, prot: "udp"))")
        let adapter = try communicator.createObjectAdapter("TestAdapter")
        _ = try adapter.add(servant: TestI(), id: Ice.stringToIdentity("test"))
        try adapter.activate()
        serverReady()
        communicator.waitForShutdown()
    }
}
