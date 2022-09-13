//
//  Tests.m
//  Tests
//
//  Created by 周亮 on 2022/9/7.
//

#import <XCTest/XCTest.h>
#include <iostream>
#import <LLDBPlugin/SBAPIPlugin.h>

using namespace lldb;

@interface LLDBPluginTests : XCTestCase

@property (nonatomic) SBDebugger debugger;
@property (nonatomic) SBTarget target;
@property (nonatomic) lldb::SBProcess process;

@end

@implementation LLDBPluginTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    SBDebugger::Initialize();
    self.debugger = lldb::SBDebugger::Create();
    self.debugger.SetAsync(false);
    self.target = self.debugger.CreateTargetWithFileAndArch("/Volumes/T7/ios/SBAPIPlugin/TestExec/a.out", "x86_64");
    self.target.BreakpointCreateByLocation("test.c", 12);
    PluginInitialize(self.debugger);
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    self.process = self.target.LaunchSimple(NULL, NULL, "/Volumes/T7/ios/SBAPIPlugin/TestExec");
    XCTAssert(self.process.GetNumThreads() > 0);
    lldb::SBThread thread = self.process.GetThreadAtIndex(0);
    auto name = thread.GetFrameAtIndex(0).GetFunctionName();
    NSLog(@"name = %s", name);
    
    
    lldb::SBCommandInterpreter interpreter = self.debugger.GetCommandInterpreter();
    SBCommandReturnObject result;
    interpreter.HandleCommand("kk find 0x100", result);
    std::cout << result.GetOutput() << std::endl;
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end

