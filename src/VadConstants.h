//
//  VadConstants.h
//  soundManagerVADRecord
//
//  Created by Roy Macdonald on 22-08-22.
//

#pragma once


enum ChannelState{
    OFX_VAD_INACTIVE = 0,
    OFX_VAD_ACTIVE,
    OFX_VAD_CHANGE_TO_ACTIVE,
    OFX_VAD_CHANGE_TO_INACTIVE
};

