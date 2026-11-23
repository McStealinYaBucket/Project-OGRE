// Base/include/rbx/RunningAverage.h: October 2013 reconstruction (Project OGRE).
// Smoothed and windowed statistics for timings, rates and counts (task scheduler, stats items,
// network peers, HTTP queues).
// Evidence: iOS 1029 symbols RunningAverage<double,double>::{RunningAverage, sample, iter},
// RunningAverage<int,double>::sample, WindowAverage<double,double>::{sample, getStats,
// getSanitizedStats, iter}, ActivityMeter<2>::updateBuckets and InvocationMeter<2>::updateBuckets;
// PLAYER RTTI for RunningAverageItem<int/double> (stats items built on RunningAverage); the
// template bodies placed in this file (TEMPLATE markers): RunningAverageTimeInterval and
// RunningAverageDutyCycle constructors, RunningAverageTimeInterval::sample,
// WindowAverage::sample<FOnBeforeDrop>, WindowAverageTimeInterval::sample,
// WindowAverageDutyCycle::sample and RunningAverageDutyCycle::dutyCycle.
// Members are declared in the order the tree had them; every class here is embedded in engine
// objects whose layouts are already matched. Dropped: TotalCountTimeInterval, ThrottlingHelper
// and BudgetedThrottlingHelper, which have no caller here and no 2013 symbol or string.
#pragma once

#include "rbx/atomic.h"
#include "rbx/boost.hpp"
#include "rbx/rbxTime.h"
#include "rbx/Debug.h"
#include "rbx/MathUtil.h"

#include <boost/static_assert.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>

#include <cmath>

namespace RBX {

// Where the 2013 engine keeps these (layouts from the matched constructors, offsets in the owner):
//   TaskScheduler        RunningAverage<int> x3 (sleeping, waiting, running job counts),
//                        RunningAverageDutyCycle<Time::Precise> schedulerDutyCycle,
//                        RunningAverage<double> averageThreadAffinity, ActivityMeter<2> per thread
//   TaskScheduler::Job   RunningAverageDutyCycle<> dutyCycle, RunningAverage<double> sleepRate and
//                        runningAverageError, WindowAverageDutyCycle<> dutyCycleWindow
//   ScriptContext        RunningAverage<double> GC interval/time/memory at 0x150/0x180/0x1b0,
//                        RunningAverageTimeInterval<> threadsResumedRate at 0x1e0, then 0x220
//   DataModel            RunningAverageTimeInterval<> osMouseMove 0x00 and mouseMove 0x40
//   Network::Peer        RunningAverageDutyCycle<> rakDutyCycle at 0xa40
//   ScriptStats          shared_ptr<ActivityMeter<2>> and shared_ptr<InvocationMeter<2>>
//   Http                 static WindowAverage<double, double> response times (Roblox, CDN)
// Those offsets fix sizeof(RunningAverage<double>) at 0x30 and sizeof(RunningAverageTimeInterval)
// at 0x40, which the member lists below reproduce.

// Exponential moving average and variance: each sample moves the mean `lerp` of the way toward
// itself (the first sample sets it outright). Non-finite samples are ignored. With bufferSize > 0
// the most recent raw samples are also kept, for iter().
template<typename ValueType = double, typename AverageType = double>
class RunningAverage
{
	boost::shared_ptr<boost::circular_buffer<ValueType> > buffer;

public:
	RunningAverage(double weight = 0.05, ValueType start = 0, unsigned int keep = 0)
		: lerp(weight), lastValue(start), mean(start), meanSquareDev(0), unsampled(true)
	{
		if (keep)
			buffer.reset(new boost::circular_buffer<ValueType>(keep));
	}

	void sample(ValueType value)
	{
		if (isFinite(value))
		{
			sampleValue(value);
			sampleVariance(value);
			if (buffer)
				buffer->push_back(value);
		}
	}

	AverageType value() const { return mean; }
	AverageType variance() const { return meanSquareDev; }
	AverageType standard_deviation() const { return std::sqrt(meanSquareDev); }
	AverageType variance_to_mean_ratio() const { return meanSquareDev / (mean * mean); }
	AverageType coefficient_of_variation() const { return standard_deviation() / mean; }
	ValueType lastSample() const { return lastValue; }

	// Visits the buffered raw samples, oldest first (nothing without a buffer).
	template<class F>
	void iter(F& f) const
	{
		typedef typename boost::circular_buffer<ValueType>::const_iterator Pos;
		if (buffer)
			for (Pos pos = buffer->begin(); pos != buffer->end(); ++pos)
				f(*pos);
	}

	// Back to the just-constructed state, centred on `to`; buffered samples are dropped too.
	void reset(ValueType to = 0)
	{
		lastValue = to;
		mean = to;
		meanSquareDev = 0;
		unsampled = true;
		if (buffer)
			buffer->clear();
	}

	bool hasSampled() { return !unsampled; }

	// Weight of each new sample, 0..1. Public and const: the duty-cycle classes read it back.
	const double lerp;

	// Layout for <double>: buffer 0x00, lerp 0x08, lastValue 0x10, mean 0x18,
	// meanSquareDev 0x20, unsampled 0x28; 0x30 in all.
private:
	ValueType lastValue;
	AverageType mean;
	AverageType meanSquareDev;
	bool unsampled;

	inline void sampleValue(ValueType value)
	{
		mean = unsampled ? value : (1.0 - lerp) * mean + lerp * (AverageType)value;
		lastValue = value;
		unsampled = false;
	}

	// Smooths the squared distance from the (already updated) mean.
	inline void sampleVariance(ValueType value)
	{
		const double delta = value - mean;
		const double squared = delta * delta;
		meanSquareDev = (1.0 - lerp) * meanSquareDev + lerp * squared;
	}
};

// Plain statistics over the last maxSamples samples, kept in a ring buffer.
template<typename ValueType = double, typename AverageType = double>
class WindowAverage
{
protected:
	boost::circular_buffer<ValueType> buffer;
	typedef typename boost::circular_buffer<ValueType>::const_iterator Pos;

public:
	struct Stats
	{
		Stats(size_t n, const AverageType& mean, const AverageType& spread)
			: samples(n), average(mean), variance(spread) {};
		size_t samples;
		AverageType average;
		AverageType variance; // sample variance (n - 1)
	};

	WindowAverage(size_t capacity) : buffer(capacity) {}
	// Resizing keeps the newest samples that still fit.
	void setMaxSamples(size_t capacity) { buffer.set_capacity(capacity); }
	size_t getMaxSamples() const { return buffer.capacity(); }

	void sample(ValueType value) { buffer.push_back(value); }

	// As sample(value), but when the buffer is full the oldest sample is first shown to
	// onBeforeDrop, which may enlarge the buffer to keep it.
	template<class F>
	void sample(ValueType value, F& onBeforeDrop)
	{
		if (buffer.full())
			onBeforeDrop(buffer.front());
		sample(value);
	}

	// Mean and variance again, leaving out the samples IsValueOutlier rejects at confidence conf
	// against the plain statistics.
	Stats getSanitizedStats(Confidence conf = C90) const
	{
		Stats all = getStats();
		if (all.samples <= 1)
			return all;

		Stats kept(0, AverageType(), AverageType());
		AverageType sd = sqrt(all.variance);

		for (Pos pos = buffer.begin(); pos != buffer.end(); ++pos)
		{
			double x = *pos;
			if (IsValueOutlier(x, all.samples, all.average, sd, conf))
				continue;
			kept.samples++;
			kept.average += x;
		}
		kept.average /= kept.samples;

		for (Pos pos = buffer.begin(); pos != buffer.end(); ++pos)
		{
			double x = *pos;
			if (IsValueOutlier(x, all.samples, all.average, sd, conf))
				continue;
			AverageType delta = (kept.average - x);
			kept.variance = kept.variance + delta * delta;
		}
		kept.variance /= (kept.samples - 1);

		return kept;
	}

	// Statistics of the newest `samples` samples (all by default), walking the ring backwards
	// from the newest entry; the variance divides by n - 1 and stays 0 for a single sample.
	Stats getStats(size_t samples = ~0) const
	{
		samples = std::min(buffer.size(), samples);
		Stats out(samples, AverageType(), AverageType());

		typename boost::circular_buffer<ValueType>::const_reverse_iterator pos;
		size_t n;
		for (pos = buffer.rbegin(), n = 0; n < samples; ++pos, ++n)
			out.average = out.average + *pos;
		if (samples != 0)
			out.average /= samples;

		for (pos = buffer.rbegin(), n = 0; n < samples; ++pos, ++n)
		{
			AverageType delta = (out.average - *pos);
			out.variance = out.variance + delta * delta;
		}
		if (samples > 1)
			out.variance /= (samples - 1);

		return out;
	}

	// newest sample, or 0
	AverageType getLatest() const { return buffer.empty() ? 0 : *(buffer.rbegin()); }

	template<class F>
	void iter(F& f) const
	{
		for (Pos pos = buffer.begin(); pos != buffer.end(); ++pos)
			f(*pos);
	}

	void clear() { buffer.clear(); }
	size_t size() const { return buffer.size(); }
};

// The interval between successive sample() calls, smoothed like RunningAverage.
template<Time::SampleMethod sampleMethod = Time::Benchmark>
class RunningAverageTimeInterval
{
public:
	RunningAverageTimeInterval(double weight = 0.05) : firstTime(true), average(weight) {}
	// TEMPLATE: PLAYER 0x009b5510
	// TEMPLATE: STUDIO 0x00a6d300
	// ??0?$RunningAverageTimeInterval@$01@RBX@@QAE@N@Z

	// The first call only starts the clock.
	void sample()
	{
		if (firstTime)
		{
			timer.reset();
			firstTime = false;
		}
		else
			average.sample(timer.reset().seconds());
	}

	// The smoothed interval, except that a wait already longer than twice it wins, so a stalled
	// task shows up before its next sample.
	Time::Interval value() const
	{
		Time::Interval sinceLast = timer.delta();
		double mean = average.value();
		if (sinceLast.seconds() > 2.0 * mean)
			return sinceLast;
		else
			return Time::Interval(mean);
	}

	// samples per second (0 before there is an interval)
	double rate() const
	{
		double seconds = value().seconds();
		return seconds > 0.0 ? 1.0 / seconds : 0.0;
	}

	double variance() const { return average.variance(); }
	double standard_deviation() const { return average.standard_deviation(); }
	double variance_to_mean_ratio() const { return average.variance_to_mean_ratio(); }
	double coefficient_of_variation() const { return average.coefficient_of_variation(); }
	double getLerp() const { return average.lerp; }
	Time::Interval lastSample() const { return Time::Interval(average.lastSample()); }

	// timer 0x00, firstTime 0x08, average 0x10 (double-aligned); 0x40 in all.
private:
	Timer<sampleMethod> timer;
	bool firstTime;
	RunningAverage<> average;
};

// WindowAverageTimeInterval's drop hook: while the buffer covers less than maxWindow of time it
// doubles the buffer instead of letting the oldest interval go; otherwise it lets it go and
// takes its length off currentWindow.
struct FOnBeforeDrop
{
	WindowAverage<>& average;
	Time::Interval& currentWindow;
	Time::Interval& maxWindow;

	FOnBeforeDrop(WindowAverage<>& window, Time::Interval& covered, Time::Interval& wanted)
		: average(window), currentWindow(covered), maxWindow(wanted) {};

	void operator()(double sample)
	{
		if (currentWindow.seconds() < maxWindow.seconds())
			average.setMaxSamples(average.getMaxSamples() * 2);
		else
			currentWindow -= Time::Interval(sample);
	}
};

// Intervals between sample() calls over a sliding window of maxWindow seconds. The buffer starts
// at 16 entries and grows geometrically until it spans the window.
template<Time::SampleMethod sampleMethod = Time::Benchmark>
class WindowAverageTimeInterval
{
public:
	WindowAverageTimeInterval(Time::Interval window) : maxWindow(window), average(16) {}

	// A zero window also shrinks the buffer back to 16.
	void setMaxWindow(Time::Interval maxWindow)
	{
		this->maxWindow = maxWindow;
		if (maxWindow.seconds() == 0.0)
			average.setMaxSamples(16);
	};
	Time::Interval getMaxWindow() const { return maxWindow; };
	size_t getCapacity() const { return average.getMaxSamples(); };

	void sample()
	{
		if (firstTime)
		{
			timer.reset();
			firstTime = false;
		}
		else
		{
			double dt = timer.reset().seconds();
			currentWindow += Time::Interval(dt);
			FOnBeforeDrop grow(average, currentWindow, maxWindow);
			average.sample(dt, grow);
		}
	}

	struct Stats
	{
		Stats(size_t n, Time::Interval mean, Time::Interval spread, Time::Interval total, double perSecond)
			: samples(n), average(mean), variance(spread), sum(total), samplespersecond(perSecond) {};
		size_t samples;
		Time::Interval average;
		Time::Interval variance;
		Time::Interval sum;      // all buffered intervals
		double samplespersecond;
	};

	// adds up what it visits
	struct FSum
	{
		FSum() : total(0.0) {};
		double total;
		void operator()(double v) { total += v; };
	};

	// samplespersecond divides the requested sample count (not the count found) by the buffered
	// time, as the tree computes it.
	Stats getStats(size_t samples = ~0) const
	{
		WindowAverage<>::Stats plain = average.getStats(samples);
		FSum sum;
		average.iter(sum);
		return Stats(plain.samples, Time::Interval(plain.average), Time::Interval(plain.variance), Time::Interval(sum.total), samples / sum.total);
	}

	template<class F>
	void iter(F& f) const { average.iter(f); }

	void clear()
	{
		firstTime = true;
		average.clear();
	}

	size_t size() const { return average.size(); }

private:
	Time::Interval currentWindow;
	Time::Interval maxWindow;
	WindowAverage<> average;
	Timer<sampleMethod> timer;
	bool firstTime; // not set by the constructor (the tree's behaviour, kept)
};

// Share of time spent inside a recurring task: smoothed busy time per step over the smoothed
// interval between steps.
template<Time::SampleMethod sampleMethod = Time::Benchmark>
class RunningAverageDutyCycle
{
public:
	RunningAverageDutyCycle(double weight) : time(weight), interval(weight) {}
	// TEMPLATE: PLAYER 0x009b5570
	// TEMPLATE: STUDIO 0x00a6d360
	// ??0?$RunningAverageDutyCycle@$01@RBX@@QAE@N@Z

	// timeBufferSize: raw busy times to keep for stepTime().iter.
	RunningAverageDutyCycle(double weight, int timeBufferSize) : time(weight, 0, timeBufferSize), interval(weight) {}

	// One step that kept the task busy for elapsedTime.
	void sample(Time::Interval elapsedTime)
	{
		interval.sample();
		time.sample(elapsedTime.seconds());
	}

	RBX::Time startSample() const { return Time::now<sampleMethod>(); }
	void stopSample(RBX::Time start) { sample(Time::now<sampleMethod>() - start); }

	// busy / interval; with no interval yet, 1 if any busy time was seen, else 0
	double dutyCycle() const
	{
		double meanInterval = interval.value().seconds();
		double meanBusy = time.value();
		return meanInterval != 0 ? meanBusy / meanInterval : (meanBusy > 0 ? 1 : 0);
	}

	double rate() const { return interval.rate(); }
	const RunningAverageTimeInterval<sampleMethod>& stepInterval() const { return interval; }
	Time::Interval lastStepInterval() { return interval.lastSample(); }
	const RunningAverage<double>& stepTime() const { return time; }
	double getIntervalLerp() const { return interval.getLerp(); }

private:
	RunningAverage<double> time;
	RunningAverageTimeInterval<sampleMethod> interval;
};

// The windowed counterpart of RunningAverageDutyCycle. The busy-time buffer is kept the same
// capacity as the interval buffer, so both cover the same steps.
template<Time::SampleMethod sampleMethod = Time::Benchmark>
class WindowAverageDutyCycle
{
public:
	WindowAverageDutyCycle(Time::Interval window) : time(16), interval(window) {}

	void setMaxWindow(Time::Interval maxWindow)
	{
		interval.setMaxWindow(maxWindow);
		if (maxWindow.seconds() == 0.0)
			time.setMaxSamples(16);
	};
	Time::Interval getMaxWindow() const { return interval.getMaxWindow(); };

	void sample(Time::Interval elapsedTime)
	{
		interval.sample();
		if (time.getMaxSamples() != interval.getCapacity())
			time.setMaxSamples(interval.getCapacity());
		time.sample(elapsedTime.seconds());
	}

	struct Stats
	{
		Stats(const typename WindowAverageTimeInterval<sampleMethod>::Stats& steps, const WindowAverage<double>::Stats& busy, double fraction)
			: interval(steps), time(busy), dutyfraction(fraction) {};
		typename WindowAverageTimeInterval<sampleMethod>::Stats interval;
		WindowAverage<double>::Stats time;
		double dutyfraction; // mean busy time over mean interval; 1.0 means always busy
	};

	// Interval and busy-time statistics over the newest `samples` steps, and their ratio (0 while
	// no interval has been measured). Feeds DataModel's job statistics ("dutyfraction").
	Stats getStats(size_t samples = ~0) const
	{
		typename WindowAverage<>::Stats busy = time.getStats(samples);
		typename WindowAverageTimeInterval<sampleMethod>::Stats steps = interval.getStats(samples);
		double interval = steps.average.seconds();
		return Stats(steps, busy, interval ? busy.average / interval : 0);
	}

	template<class F>
	void iterTimes(F& f) const { time.iter(f); }
	template<class F>
	void iterIntervals(F& f) const { interval.iter(f); }

	// counts the visited values above gtValue
	struct GTCounter
	{
		GTCounter(double gt) : hits(0), threshold(gt) {};
		size_t hits;
		double threshold;
		void operator()(double dt)
		{
			if (dt > threshold)
				hits++;
		}
	};

	size_t countTimesGreaterThan(Time::Interval dt) const
	{
		GTCounter above(dt.seconds());
		iterTimes(above);
		return above.hits;
	}

	size_t countIntervalsGreaterThan(Time::Interval dt) const
	{
		GTCounter above(dt.seconds());
		iterIntervals(above);
		return above.hits;
	}

	void clear()
	{
		time.clear();
		interval.clear();
	}

	size_t timesamples() const { return time.size(); }
	size_t intervalsamples() const { return interval.size(); }

private:
	WindowAverage<double> time;
	WindowAverageTimeInterval<sampleMethod> interval;
};

// Lock-free average level of a counter over the last windowSeconds seconds. Time is cut into
// 1024 buckets per second, each remembering the counter's value when it was last passed;
// totalValue is the running sum of all buckets, maintained one unit at a time with atomics.
template<int windowSeconds>
class ActivityMeter
{
	static const int bucketCount = windowSeconds * 1024;
	boost::array<char, bucketCount> buckets;
	rbx::atomic<int> currentTime;
	rbx::atomic<int> currentValue;
	rbx::atomic<int> totalValue;
	RBX::Time startTime;
	RBX::Time lastSampleTime;

public:
	ActivityMeter() : currentTime(-1), currentValue(0), totalValue(0), startTime(RBX::Time::now<Time::Fast>())
	{
		for (size_t b = 0; b < buckets.size(); ++b)
			buckets[b] = 0;
	}

	// mean counter value over the window
	double averageValue()
	{
		updateBuckets();
		return (double)totalValue / (double)bucketCount;
	}

	void increment()
	{
		updateBuckets();
		++currentValue;
	}

	void decrement()
	{
		updateBuckets();
		--currentValue;
	}

	// Writes the current value into every bucket passed since the last update. currentTime.swap
	// hands each bucket range to exactly one caller, so concurrent updates do not double count;
	// a bucket is a char, so levels above 127 do not fit (2013's callers count running scripts
	// and busy threads, far below that).
	void updateBuckets()
	{
		RBX::Time now = Time::now<Time::Fast>();
		if (lastSampleTime == now)
			return;

		lastSampleTime = now;
		unsigned long nowBucket = ((unsigned long)(bucketCount * (lastSampleTime - startTime).seconds()));
		unsigned long lastBucket = currentTime.swap(nowBucket);
		if (lastBucket < nowBucket)
		{
			long level = currentValue;
			for (unsigned long t = lastBucket + 1; t <= nowBucket; ++t)
			{
				int slot = t % bucketCount;
				int stale = buckets[slot];
				for (int k = 0; k < stale; ++k)
					--totalValue;
				buckets[slot] = (char)level;
				for (int k = 0; k < level; ++k)
					++totalValue;
			}
		}
	}
};

// Lock-free count of increment() calls over the last windowSeconds seconds, in the same 1024
// buckets per second as ActivityMeter (a bucket records at most one call).
template<int windowSeconds>
class InvocationMeter
{
	static const int bucketCount = windowSeconds * 1024;
	boost::array<char, bucketCount> buckets;
	rbx::atomic<int> currentTime;
	rbx::atomic<int> totalValue;
	RBX::Time startTime;
	RBX::Time lastSampleTime;

public:
	InvocationMeter() : currentTime(-1), totalValue(0), startTime(RBX::Time::now<Time::Fast>())
	{
		for (size_t b = 0; b < buckets.size(); ++b)
			buckets[b] = 0;
	}

	// calls per second, averaged over the window (integer division, as the tree has it)
	double getTotalValuePerSecond()
	{
		updateBuckets(false);
		return totalValue / windowSeconds;
	}

	void increment() { updateBuckets(true); }

	// Clears the buckets passed since the last update, then marks the current one if counting.
	void updateBuckets(bool increment)
	{
		RBX::Time now = Time::now<Time::Fast>();
		if (lastSampleTime == now)
			return;

		lastSampleTime = now;
		unsigned long nowBucket = ((unsigned long)(bucketCount * (lastSampleTime - startTime).seconds()));
		unsigned long lastBucket = currentTime.swap(nowBucket);
		if (lastBucket < nowBucket)
		{
			for (unsigned long t = lastBucket + 1; t <= nowBucket; ++t)
			{
				int slot = t % bucketCount;
				int stale = buckets[slot];
				for (int k = 0; k < stale; ++k)
					--totalValue;
				buckets[slot] = 0;
			}
		}
		if (increment)
		{
			int mark = 1;
			int slot = nowBucket % bucketCount;
			buckets[slot] = mark;
			for (int k = 0; k < mark; ++k)
				++totalValue;
		}
	}
};

}

// TEMPLATE: PLAYER 0x009b6300
// TEMPLATE: STUDIO 0x00a6e870
// ?sample@?$RunningAverageTimeInterval@$01@RBX@@QAEXXZ

// TEMPLATE: PLAYER 0x009ba900
// TEMPLATE: STUDIO 0x00a71750
// ??$sample@UFOnBeforeDrop@RBX@@@?$WindowAverage@NN@RBX@@QAEXNAAUFOnBeforeDrop@1@@Z

// TEMPLATE: PLAYER 0x009ba9d0
// TEMPLATE: STUDIO 0x00a71820
// ?sample@?$WindowAverageTimeInterval@$00@RBX@@QAEXXZ

// TEMPLATE: PLAYER 0x009baba0
// TEMPLATE: STUDIO 0x00a719f0
// ?sample@?$WindowAverageDutyCycle@$00@RBX@@QAEXVInterval@Time@2@@Z

// TEMPLATE: PLAYER 0x009b3980
// TEMPLATE: STUDIO 0x00a6bab0
// ?dutyCycle@?$RunningAverageDutyCycle@$01@RBX@@QBENXZ
