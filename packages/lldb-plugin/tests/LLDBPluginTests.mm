//
//  Tests.m
//  Tests
//
//  Created by 周亮 on 2022/9/7.
//

#import <LLDBPlugin/SBAPIPlugin.h>
#import <XCTest/XCTest.h>
#include <iostream>
#include <string>

template <typename... Args>
auto string_format(const std::string& format, Args... args) -> std::string {
    size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args...);  // Extra space for \0
    // unique_ptr<char[]> buf(new char[size]);
    char bytes[size];
    snprintf(bytes, size, format.c_str(), args...);
    return {bytes};
}

using namespace lldb;

@interface LLDBPluginTests : XCTestCase

@property(nonatomic) SBDebugger debugger;
@property(nonatomic) SBTarget target;
@property(nonatomic) lldb::SBProcess process;

@end

@implementation LLDBPluginTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    SBDebugger::Initialize();
    self.debugger = lldb::SBDebugger::Create();
    self.debugger.SetAsync(false);
    self.target = self.debugger.CreateTargetWithFileAndArch("/Volumes/T7/ios/SBAPIPlugin/TestExec/a.out", "arm64");
    self.target.BreakpointCreateByLocation("test.c", 23);
    PluginInitialize(self.debugger);
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    self.process = self.target.LaunchSimple(nullptr, nullptr, "/Volumes/T7/ios/SBAPIPlugin/TestExec");
    XCTAssert(self.process.GetNumThreads() > 0);
    lldb::SBThread thread = self.process.GetThreadAtIndex(0);
    auto name = thread.GetFrameAtIndex(0).GetFunctionName();
    NSLog(@"name = %s", name);

    lldb::SBCommandInterpreter interpreter = self.debugger.GetCommandInterpreter();
    SBCommandReturnObject result;
    interpreter.HandleCommand("po p", result);
    auto cmd = string_format("kk find %s", result.GetOutput());
    auto loopupCmd = string_format("image loo -a %s", result.GetOutput());
    interpreter.HandleCommand(loopupCmd.c_str(), result);
    std::cout << result.GetOutput() << std::endl;
    interpreter.HandleCommand(cmd.c_str(), result);
    std::cout << result.GetOutput() << std::endl;
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
