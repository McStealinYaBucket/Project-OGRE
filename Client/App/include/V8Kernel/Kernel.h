/* V8Kernel/Kernel.h -- October 2013 (pipeline terminal, 0x30).
   Evidence: C2 0x52A20E (296B: +4 upstream / +8 downstream 0 [terminal] /
     +12 0 / +16 inStepCode false / +20..+32 zero / +36 news 0xB0 KernelData /
     +40/+44 news 0xA01C CodeProfilers + numKernels++) / D2 (batch221b:
     :41 !inStepCode + delete KernelData + profiler deletes + delete +8) /
     getStageType 0x52E950 -> 15 / getKernel 0x52E95A returns this
     (batches 213/218/221b/222).
   Vtable proof (batch225 @0xEED8C0): [2]D1 [3]D0 [4]getStageType
   [5]getKernel -- Kernel : IStage directly (no onEdge slots; [6][7] zero).
   Members +20/+24/+28/+32 (C2 zero, untouched by D2): identity queued
   with the stepper flow (section 123+); modeled as ints.
   Profiler DELTA (StepJoints precedent): binary news 2x CodeProfiler
   ("Kernel Bodies"/"Kernel Connectors"); module absent, always NULL.
   Original file: Client/App/include/V8Kernel/Kernel.h (path inferred) */

#pragma once

#include "V8Kernel/IStage.h"

namespace RBX {

	class KernelData;
	class Point;
	class Body;
	class Connector;

	class Kernel : public IStage
	{
	public:
		Kernel(IStage* upstream);
		virtual ~Kernel();

		// mac: 0x52E950 (10B): KERNEL_STAGE.
		// FUNCTION: PLAYER 0x0082f750
		// FUNCTION: STUDIO 0x009a1d00
		virtual StageType getStageType() const { return KERNEL_STAGE; }
		// mac: 0x52E95A (8B): returns this (terminal).
		// FUNCTION: PLAYER 0x0079f710
		// FUNCTION: STUDIO 0x007748a0
		virtual Kernel* getKernel() { return this; }

		static int numKernels;

		// mac: 0x52A7F4 (batch244: :98 !inStepCode + :60/:61 IndexArray
		// protocol into data->points; removePoint/deletePoint queue with
		// the point-removal body).
		void insertPoint(Point* point);
		// 0x52Axxx (batch222: :77 root-dt assert + KernelData::insertBody;
		// the +12 max-count maintenance is skipped (binary repurposes
		// Kernel+12 as a counter; ours stays NULL-world, unread -- loud
		// DELTA, queued with the stepper stats).
		void insertBody(Body* body);
		// (batch222: :88 !inStepCode + :90 dt-consistency arms +
		// KernelData::removeBody).
		void removeBody(Body* body);
		// mac: 0x52A9AE (batch249: :110 !inStepCode + fastRemove by index).
		void removePoint(Point* point);
		// mac: 0x52AB7E (batch249: body+localPos match bump owners + delete
		// the input; else insertPoint; :103 index asserts).
		Point* searchForDuplicatePoint(Point* input);
		// (batch244: post-dec equals 1 releases the last owner, then
		// removePoint + delete).
		void deletePoint(Point* point);
		// (batch222: :104 !inStepCode + delegate; returns the kind).
		int insertConnector(Connector* connector);
		// (batch222: :116 !inStepCode + delegate).
		void removeConnector(Connector* connector);
		// 2016 shapes (first-party; stepWorldThrottled validates each
		// realtime connector before solving; EAs queued).
		bool validateConnector(Connector* connector) const;
		bool validateConnectorBody(Body* body) const;
		// mac: 0x52BBDA / 0x52B058 (batch298: kernel stepping loops).
		void stepWorld();
		void stepWorldThrottled();

		// Array-size counters (batch222: direct data->list.size() reads).
		int numFreeFallBodies() const;
		int numRealTimeBodies() const;
		int numJointBodies() const;
		int numContactBodies() const;
		int numLeafBodies() const;
		// Sum of secondPass + realTime + joint + contact (humanoid
		// excluded -- disasm-pinned data offsets +116/+132/+148/+164).
		int numConnectors() const;
		int numJointConnectors() const;

	private:
		bool inStepCode;      // +16 (C2 false; D2 :41 assert).
		int numLastIterations;    // +20 (stepWorld impulse stats).
		int numOfMaxIterations;   // +24 (stepWorld impulse stats).
		float error;              // +28 (stepWorld impulse stats).
		float maxError;           // +32 (stepWorld impulse stats).
		KernelData* data;     // +36 (C2 news 0xB0).
		void* profilerBodies;     // +40 (always NULL -- see note).
		void* profilerConnectors; // +44 (always NULL -- see note).
	};

} // namespace RBX
