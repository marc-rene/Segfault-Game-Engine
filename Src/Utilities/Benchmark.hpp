#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <fstream>


namespace ENGINE
{
	namespace Util
	{
		namespace AI_Generated
		{
			using clock = std::chrono::steady_clock;
			using ns = std::chrono::nanoseconds;

			struct Result {
				std::string name;
				std::size_t runs = 0;       // number of timed runs
				std::size_t iters_per_run;  // iterations per run (batch size)
				double ns_per_op = 0.0;
				double ops_per_sec = 0.0;
				double min_ns = 0.0, med_ns = 0.0, p95_ns = 0.0, max_ns = 0.0; // per-op stats
			};

			template <typename T>
			inline void DoNotOptimise(const T& value) {

				// Prevent optimisation and reordering
				std::atomic_signal_fence(std::memory_order_seq_cst);
				(void)*reinterpret_cast<volatile const char*>(&value);
				std::atomic_signal_fence(std::memory_order_seq_cst);

			}

			inline void ClobberMemory() {

				std::atomic_signal_fence(std::memory_order_seq_cst);

			}

			template <typename F>
			Result bench(const std::string& name, F&& fn,
				std::size_t warmup_runs = 3,
				std::size_t iters_per_run = 10000,
				std::size_t timed_runs = 10) {
				// Warm-up (JIT-like effects, caches, branch predictors)
				for (std::size_t w = 0; w < warmup_runs; ++w) {
					for (std::size_t i = 0; i < iters_per_run; ++i) {
						auto v = fn();
						DoNotOptimise(v);
					}
					ClobberMemory();
				}

				std::vector<double> per_op_ns;
				per_op_ns.reserve(timed_runs);

				for (std::size_t r = 0; r < timed_runs; ++r) {
					auto t0 = clock::now();
					for (std::size_t i = 0; i < iters_per_run; ++i) {
						auto v = fn();
						DoNotOptimise(v);
					}
					ClobberMemory();
					auto t1 = clock::now();
					const auto run_ns = std::chrono::duration_cast<ns>(t1 - t0).count();
					per_op_ns.push_back(static_cast<double>(run_ns) / static_cast<double>(iters_per_run));
				}

				std::sort(per_op_ns.begin(), per_op_ns.end());

				auto pct = [&](double p) {
					if (per_op_ns.empty()) return 0.0;
					double idx = p * (per_op_ns.size() - 1);
					std::size_t lo = static_cast<std::size_t>(idx);
					std::size_t hi = std::min(lo + 1, per_op_ns.size() - 1);
					double t = idx - lo;
					return per_op_ns[lo] * (1.0 - t) + per_op_ns[hi] * t;
					};

				double mean = std::accumulate(per_op_ns.begin(), per_op_ns.end(), 0.0) / per_op_ns.size();
				double ns_op = mean;
				double ops_s = ns_op > 0.0 ? 1e9 / ns_op : 0.0;

				Result res;
				res.name = name;
				res.runs = timed_runs;
				res.iters_per_run = iters_per_run;
				res.ns_per_op = ns_op;
				res.ops_per_sec = ops_s;
				res.min_ns = per_op_ns.front();
				res.med_ns = pct(0.5);
				res.p95_ns = pct(0.95);
				res.max_ns = per_op_ns.back();
				return res;
			}

			inline void print(const Result& r) {
				auto pad = [](const std::string& s, int w) { return s + std::string(std::max(0, w - (int)s.size()), ' '); };
				std::cout << pad(r.name, 22)
					<< " runs=" << r.runs
					<< " batch=" << r.iters_per_run
					<< "  ns/op(avg)=" << std::fixed << std::setprecision(2) << r.ns_per_op
					<< "  ops/s≈" << std::setprecision(0) << r.ops_per_sec
					<< "  [min/med/p95/max ns]=["
					<< std::setprecision(2) << r.min_ns << "/"
					<< r.med_ns << "/"
					<< r.p95_ns << "/"
					<< r.max_ns << "]\n";
			}

			inline void save_to_file(const std::string& filename, Result& result) {
				std::ofstream file(filename);
				if (!file.is_open()) {
					std::cerr << "⚠️  Couldn't open file '" << filename << "' for writing.\n";
					return;
				}

				file << "microbench results\n";
				file << "==================\n\n";
				
					file << result.name << "\n"
						<< "  runs=" << result.runs
						<< "  batch=" << result.iters_per_run
						<< "  ns/op(avg)=" << std::fixed << std::setprecision(2) << result.ns_per_op
						<< "  ops/s≈" << std::setprecision(0) << result.ops_per_sec << "\n"
						<< "  [min/med/p95/max ns]=["
						<< std::setprecision(2)
						<< result.min_ns << "/" << result.med_ns << "/" << result.p95_ns << "/" << result.max_ns << "]\n\n";
				

				file.close();
				std::cout << "✅ Saved results to '" << filename << "'\n";
			}

		}
	}
} // namespace GPT-QuickBenchmark
