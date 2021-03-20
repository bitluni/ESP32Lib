#pragma once

//== VGA ==
#include <VGA/VGAMode.h>

//DMA-only drivers
#include <VGA/VGA14Bit.h>
#include <VGA/VGA6Bit.h>
#include <VGA/VGA3Bit.h>
#include <VGA/VGA8BitDAC.h>
//Interrupt-based drivers
#include <VGA/VGA14BitI.h>
#include <VGA/VGA6BitI.h>
#include <VGA/VGA3BitI.h>
#include <VGA/VGA8BitDACI.h>
#include <VGA/VGA1BitI.h>
#include <VGA/VGATextI.h>

//== COMPOSITE ==
#include <Composite/CompMode.h>

//DMA-only drivers
#include <Composite/CompositeGrayDAC.h>
#include <Composite/CompositeGrayLadder.h>
#include <Composite/CompositeColorDAC.h>
#include <Composite/CompositeColorLadder.h>
#include <Composite/CompositeColorDACMemory.h>
#include <Composite/CompositeColorLadderMemory.h>
#include <Composite/CompositeGrayPDM8.h>
#include <Composite/CompositeGrayPDM4.h>
#include <Composite/CompositeGrayPDM2.h>
//Interrupt-based drivers
#include <Composite/CompositeGrayDACI.h>
#include <Composite/CompositeGrayLadderI.h>

#include <LED/SerialLED.h>
#include <LED/ParallelLED.h>
#include <LED/ParallelLEDGraphics.h>

#include <Graphics/Sprites.h>
#include <Graphics/Mesh.h>
#include <Graphics/Sprites.h>
#include <Graphics/Animation.h>
