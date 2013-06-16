/*
 * Copyright (C) 2013 SheenFigure
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "SSRenderOperation.h"

@implementation SSRenderOperation {
    SSOperationExecutionBlock _executionBlock;
}

- (id)initWithExecutionBlock:(SSOperationExecutionBlock)block {
    self = [super init];
    if (self) {
        _executionBlock = SS_COPY(block);
    }
    
    return self;
}

+ (SSRenderOperation *)operationWithExecutionBlock:(SSOperationExecutionBlock)block {
    return SS_AUTORELEASE([[SSRenderOperation alloc] initWithExecutionBlock:block]);
}

- (void)main {
    if (!_cancel) {
        _executionBlock(SS_AUTORELEASE(SS_RETAIN(self)));
    }
}

- (void)cancel {
    [super cancel];
    _cancel = YES;
}

- (BOOL)isCancelled {
    return _cancel;
}

- (void)dealloc {
    SS_RELEASE(_executionBlock);
    
#ifndef SS_ARC_ENABLED
    [super dealloc];
#endif
}

@end
