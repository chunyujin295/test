










#ifndef _AI3D_OPTIM_LORANSAC_H_
#define _AI3D_OPTIM_LORANSAC_H_

#include <cfloat>
#include <random>
#include <stdexcept>
#include <vector>

#include "Core/RandomSampler.h"
#include "Core/Ransac.h"
#include "Core/SupportMeasurement.h"
#include "Core/alignment.h"


namespace AI3D {
    namespace CORE {

        
        
        
        template <typename Estimator, typename LocalEstimator,
            typename SupportMeasurer = InlierSupportMeasurer,
            typename Sampler = RandomSampler>
            class LORANSAC : public RANSAC<Estimator, SupportMeasurer, Sampler> {
            public:
                using typename RANSAC<Estimator, SupportMeasurer, Sampler>::Report;

                explicit LORANSAC(const RANSACOptions& options);

                
                
                
                
                
                
                Report Estimate(const std::vector<typename Estimator::X_t>& X,
                    const std::vector<typename Estimator::Y_t>& Y);

                
                using RANSAC<Estimator, SupportMeasurer, Sampler>::estimator;
                LocalEstimator local_estimator;
                using RANSAC<Estimator, SupportMeasurer, Sampler>::sampler;
                using RANSAC<Estimator, SupportMeasurer, Sampler>::support_measurer;

            private:
                using RANSAC<Estimator, SupportMeasurer, Sampler>::options_;
        };

        
        
        

        template <typename Estimator, typename LocalEstimator, typename SupportMeasurer,
            typename Sampler>
            LORANSAC<Estimator, LocalEstimator, SupportMeasurer, Sampler>::LORANSAC(
                const RANSACOptions& options)
            : RANSAC<Estimator, SupportMeasurer, Sampler>(options) {}

        template <typename Estimator, typename LocalEstimator, typename SupportMeasurer,
            typename Sampler>
            typename LORANSAC<Estimator, LocalEstimator, SupportMeasurer, Sampler>::Report
            LORANSAC<Estimator, LocalEstimator, SupportMeasurer, Sampler>::Estimate(
                const std::vector<typename Estimator::X_t>& X,
                const std::vector<typename Estimator::Y_t>& Y) {
            

            const size_t num_samples = X.size();

            typename RANSAC<Estimator, SupportMeasurer, Sampler>::Report report;
            report.success = false;
            report.num_trials = 0;

            if (num_samples < Estimator::kMinNumSamples) {
                return report;
            }

            typename SupportMeasurer::Support best_support;
            typename Estimator::M_t best_model;
            bool best_model_is_local = false;

            bool abort = false;

            const double max_residual = options_.max_error * options_.max_error;

            std::vector<double> residuals;
            std::vector<double> best_local_residuals;

            std::vector<typename LocalEstimator::X_t> X_inlier;
            std::vector<typename LocalEstimator::Y_t> Y_inlier;

            std::vector<typename Estimator::X_t> X_rand(Estimator::kMinNumSamples);
            std::vector<typename Estimator::Y_t> Y_rand(Estimator::kMinNumSamples);

            sampler.Initialize(num_samples);

            size_t max_num_trials = options_.max_num_trials;
            max_num_trials = std::min<size_t>(max_num_trials, sampler.MaxNumSamples());
            size_t dyn_max_num_trials = max_num_trials;

            for (report.num_trials = 0; report.num_trials < max_num_trials;
                ++report.num_trials) {
                if (abort) {
                    report.num_trials += 1;
                    break;
                }

                sampler.SampleXY(X, Y, &X_rand, &Y_rand);

                
                const std::vector<typename Estimator::M_t> sample_models =
                    estimator.Estimate(X_rand, Y_rand);

                
                for (const auto& sample_model : sample_models) {
                    estimator.Residuals(X, Y, sample_model, &residuals);
                    

                    const auto support = support_measurer.Evaluate(residuals, max_residual);

                    
                    if (support_measurer.Compare(support, best_support)) {
                        best_support = support;
                        best_model = sample_model;
                        best_model_is_local = false;

                        
                        if (support.num_inliers > Estimator::kMinNumSamples &&
                            support.num_inliers >= LocalEstimator::kMinNumSamples) {
                            
                            const size_t kMaxNumLocalTrials = 10;
                            for (size_t local_num_trials = 0;
                                local_num_trials < kMaxNumLocalTrials; ++local_num_trials) {
                                X_inlier.clear();
                                Y_inlier.clear();
                                X_inlier.reserve(num_samples);
                                Y_inlier.reserve(num_samples);
                                for (size_t i = 0; i < residuals.size(); ++i) {
                                    if (residuals[i] <= max_residual) {
                                        X_inlier.push_back(X[i]);
                                        Y_inlier.push_back(Y[i]);
                                    }
                                }

                                const std::vector<typename LocalEstimator::M_t> local_models =
                                    local_estimator.Estimate(X_inlier, Y_inlier);

                                const size_t prev_best_num_inliers = best_support.num_inliers;

                                for (const auto& local_model : local_models) {
                                    local_estimator.Residuals(X, Y, local_model, &residuals);
                                    

                                    const auto local_support =
                                        support_measurer.Evaluate(residuals, max_residual);

                                    
                                    if (support_measurer.Compare(local_support, best_support)) {
                                        best_support = local_support;
                                        best_model = local_model;
                                        best_model_is_local = true;
                                        std::swap(residuals, best_local_residuals);
                                    }
                                }

                                
                                
                                if (best_support.num_inliers <= prev_best_num_inliers) {
                                    break;
                                }

                                
                                
                                std::swap(residuals, best_local_residuals);
                            }
                        }

                        dyn_max_num_trials =
                            RANSAC<Estimator, SupportMeasurer, Sampler>::ComputeNumTrials(
                                best_support.num_inliers, num_samples, options_.confidence,
                                options_.dyn_num_trials_multiplier);
                    }

                    if (report.num_trials >= dyn_max_num_trials &&
                        report.num_trials >= options_.min_num_trials) {
                        abort = true;
                        break;
                    }
                }
            }

            report.support = best_support;
            report.model = best_model;

            
            if (report.support.num_inliers < estimator.kMinNumSamples) {
                return report;
            }

            report.success = true;

            
            
            

            if (best_model_is_local) {
                local_estimator.Residuals(X, Y, report.model, &residuals);
            }
            else {
                estimator.Residuals(X, Y, report.model, &residuals);
            }

            

            report.inlier_mask.resize(num_samples);
            for (size_t i = 0; i < residuals.size(); ++i) {
                report.inlier_mask[i] = residuals[i] <= max_residual;
            }

            return report;
        }

    }  
}
#endif  
