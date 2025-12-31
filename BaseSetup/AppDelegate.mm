//
//  AppDelegate.m
//  BaseSetup
//
//  Created by Robert Rowe on 12/30/25.
//

#import  "AppDelegate.h"
#include "Pfx.hpp"
#include "BaseSetup.hpp"

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

Pfx*       p = nullptr;
BaseSetup* b = nullptr;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    p = new Pfx();
    if (!p)
    {
        NSLog(@"ERROR: can't get audio IO");
        exit(1);
    }
    b = new BaseSetup();
    p->SetScore(b);
    p->Start();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    p->Stop();
    delete b;
    delete p;
    p = nullptr;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app { return YES; }

@end
