//
//  AvidaEDPopViewStatView.h
//  avida/apps/viewer-macos
//
//  Created by David Bryson on 4/15/11.
//  Copyright 2011 Michigan State University. All rights reserved.
//  http://avida.devosoft.org/viewer-macos
//
//  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
//  following conditions are met:
//  
//  1.  Redistributions of source code must retain the above copyright notice, this list of conditions and the
//      following disclaimer.
//  2.  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//      following disclaimer in the documentation and/or other materials provided with the distribution.
//  3.  Neither the name of Michigan State University, nor the names of contributors may be used to endorse or promote
//      products derived from this software without specific prior written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY MICHIGAN STATE UNIVERSITY AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL MICHIGAN STATE UNIVERSITY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  Authors: David M. Bryson <david@programerror.com>
//

#import <Cocoa/Cocoa.h>

#include "avida/data/Recorder.h"

#import "OrgColorBox.h"

@class AvidaRun;
class AvidaEDPopViewStatViewRecorder;
@class AvidaEDPopViewStatViewEnvActions;


@interface AvidaEDPopViewStatView : NSView {
  IBOutlet NSView* popStatsView;
  IBOutlet NSTextField* txtPopSize;
  IBOutlet NSTextField* txtFitness;
  IBOutlet NSTextField* txtMetabolicRate;
  IBOutlet NSTextField* txtGestation;
  IBOutlet NSTextField* txtAge;
  IBOutlet NSTableView* tblEnvActions;
  
  IBOutlet NSView* orgStatsView;  
  IBOutlet NSTextField* txtOrgName;
  IBOutlet OrgColorBox* boxOrgColor;
  IBOutlet NSTextField* txtOrgFitness;
  IBOutlet NSTextField* txtOrgMetabolicRate;
  IBOutlet NSTextField* txtOrgGestation;
  IBOutlet NSTextField* txtOrgAge;
  IBOutlet NSTextField* txtOrgAncestor;
  IBOutlet NSTableView* tblOrgEnvActions;
  
  AvidaRun* run;
  Avida::Data::RecorderPtr recorder;
  AvidaEDPopViewStatViewEnvActions* envActions;
  AvidaEDPopViewStatViewEnvActions* orgEnvActions;
  IBOutlet id envActionChangeDelegate;
}

- (id) initWithFrame:(NSRect)frame;
- (void) awakeFromNib;

- (void) dealloc;
- (void) finalize;

- (BOOL) isFlipped;

- (void) resizeSubviewsWithOldSize:(NSSize)oldBoundsSize;

- (void) setAvidaRun:(AvidaRun*)avidarun;
- (void) clearAvidaRun;

- (void) clearSelectedOrg;

@end


class AvidaEDPopViewStatViewRecorder : public Avida::Data::Recorder
{
private:
  Avida::Data::ConstDataSetPtr m_requested;
  AvidaEDPopViewStatView* m_view;
  
public:
  AvidaEDPopViewStatViewRecorder(AvidaEDPopViewStatView* view) : m_view(view) { ; }
  
  Avida::Data::ConstDataSetPtr GetRequested();
  void NotifyData(Avida::Update, Avida::Data::DataRetrievalFunctor retrieve_data);
};

