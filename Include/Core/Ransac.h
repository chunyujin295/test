

#ifndef _AI3D_OPTIM_RANSAC_H_
#define _AI3D_OPTIM_RANSAC_H_

#include <cfloat>
#include <random>
#include <stdexcept>
#include <vector>



namespace AI3D {
    namespace CORE {
        struct RANSACOptions {
            
            
            double max_error = 0.0;

            
            
            double min_inlier_ratio = 0.1;

            
            
            double confidence = 0.99;

            
            
            double dyn_num_trials_multiplier = 3.0;

            
            size_t min_num_trials = 0;
            size_t max_num_trials = std::numeric_limits<size_t>::max();

            void Check() const {
                
            }
        };

        template <typename Estimator, typename SupportMeasurer = InlierSupportMeasurer,
            typename Sampler = RandomSampler>
            class RANSAC {
            public:
                struct Report {
                    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

                        
                        bool success = false;

                    
                    size_t num_trials = 0;

                    
                    typename SupportMeasurer::Support support;

                    
                    std::vector<char> inlier_mask;

                    
                    typename Estimator::M_t model;
                };

                explicit RANSAC(const RANSACOptions& options);

                
                
                
                
                
                
                
                
                
                
                
                
                static size_t ComputeNumTrials(const size_t num_inliers,
                    const size_t num_samples,
                    const double confidence,
                    const double num_trials_multiplier);

                
                
                
                
                
                
                Report Estimate(const std::vector<typename Estimator::X_t>& X,
                    const std::vector<typename Estimator::Y_t>& Y);

                
                
                Estimator estimator;
                Sampler sampler;
                SupportMeasurer support_measurer;

            protected:
                RANSACOptions options_;
        };

        
        
        

        template <typename Estimator, typename SupportMeasurer, typename Sampler>
        RANSAC<Estimator, SupportMeasurer, Sampler>::RANSAC(
            const RANSACOptions& options)
            : sampler(Sampler(Estimator::kMinNumSamples)), options_(options) {
            options.Check();

            
            const size_t kNumSamples = 100000;
            const size_t dyn_max_num_trials = ComputeNumTrials(
                static_cast<size_t>(options_.min_inlier_ratio * kNumSamples), kNumSamples,
                options_.confidence, options_.dyn_num_trials_multiplier);
            options_.max_num_trials =
                std::min<size_t>(options_.max_num_trials, dyn_max_num_trials);
        }

        template <typename Estimator, typename SupportMeasurer, typename Sampler>
        size_t RANSAC<Estimator, SupportMeasurer, Sampler>::ComputeNumTrials(
            const size_t num_inliers, const size_t num_samples, const double confidence,
            const double num_trials_multiplier) {
            const double inlier_ratio = num_inliers / static_cast<double>(num_samples);

            const double nom = 1 - confidence;
            if (nom <= 0) {
                return std::numeric_limits<size_t>::max();
            }

            const double denom = 1 - std::pow(inlier_ratio, Estimator::kMinNumSamples);
            if (denom <= 0) {
                return 1;
            }
            
            if (denom == 1.0) {
                return std::numeric_limits<size_t>::max();
            }

            return static_cast<size_t>(
                std::ceil(std::log(nom) / std::log(denom) * num_trials_multiplier));
        }

        template <typename Estimator, typename SupportMeasurer, typename Sampler>
        typename RANSAC<Estimator, SupportMeasurer, Sampler>::Report
            RANSAC<Estimator, SupportMeasurer, Sampler>::Estimate(
                const std::vector<typename Estimator::X_t>& X,
                const std::vector<typename Estimator::Y_t>& Y) {
            

            const size_t num_samples = X.size();

            Report report;
            report.success = false;
            report.num_trials = 0;

            if (num_samples < Estimator::kMinNumSamples) {
                return report;
            }

            typename SupportMeasurer::Support best_support;
            typename Estimator::M_t best_model;

            bool abort = false;

            const double max_residual = options_.max_error * options_.max_error;

            std::vector<double> residuals(num_samples);

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

                        dyn_max_num_trials = ComputeNumTrials(
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

            
            
            

            estimator.Residuals(X, Y, report.model, &residuals);
            

            report.inlier_mask.resize(num_samples);
            for (size_t i = 0; i < residuals.size(); ++i) {
                report.inlier_mask[i] = residuals[i] <= max_residual;
            }

            return report;
        }
    }
}  

#endif  
