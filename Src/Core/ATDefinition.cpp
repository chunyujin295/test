
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>
#include <omp.h>

#include "Core/ATDefinition.h"
#include "Core/ATCommandSet.h"
namespace AI3D
{
    namespace CORE
    {


       

            ATDefinition::ATDefinition(const ATData& Atdata,bool hastiepoints)
            {
                atdata_ = Atdata;
                hastiepoints_ = hastiepoints;
            };

            int ATDefinition::GetPolicyDim(policies_e object)
            {
                switch (object)
                {
                case AI3D::CORE::POLICIES_COMPUTE:
                    return 1;
                    break;
                case AI3D::CORE::POLICIES_ADJUST:
                    return 1;
                    break;
                case AI3D::CORE::POLICIES_KEEP:
                    return 1;
                    break;
                case AI3D::CORE::POLICIES_COMPUTE_ADJUST:
                    return 2;
                    break;
                case AI3D::CORE::POLICIES_COMPUTE_ADJUST_KEEP:
                    return 3;
                    break;
                case AI3D::CORE::POLICIES_COMPUTE_KEEP:
                    return 2;
                    break;
                case AI3D::CORE::POLICIES_ADJUST_KEEP:
                    return 2;
                    break;
                default:
                    break;
                }
            }

            
            std::tuple<policies_e, std::vector<std::pair<policies_e, bool>>> ATDefinition::GetPolicy(policies_object_e object)
            {
                bool bhastiepoints = hastiepoints_ || atdata_.HasTiepoints();
                policies_e policeforuse = POLICIES_ADJUST_KEEP;
                policies_e defaultpolice = POLICIES_COMPUTE;
                int num = GetPolicyDim(policeforuse);
                std::vector<std::pair<policies_e, bool>>  vecitem;
                if (object == PO_OBJ_TIEPOINTS)
                {
                    
                    
                    policeforuse = POLICIES_COMPUTE_ADJUST_KEEP;
                    defaultpolice = POLICIES_COMPUTE;
                    bool cancompute = true;
                    bool cankeep = bhastiepoints && (atdata_.GetATCompleteStatus() != AT_complete_status_e::INCOMPLETE_PHOTOS);
                    bool canadjust = cankeep ;

                    num = GetPolicyDim(policeforuse);
                    vecitem.resize(3);
                    vecitem[0].first = POLICIES_COMPUTE;
                    vecitem[1].first = POLICIES_ADJUST;
                    vecitem[2].first = POLICIES_KEEP;
                    vecitem[0].second = true;
                    vecitem[1].second = canadjust;
                    vecitem[2].second = cankeep;
                    if (canadjust)
                    {
                        defaultpolice = POLICIES_ADJUST;

                    }


                    if (!cankeep)
                    {
                        defaultpolice = POLICIES_COMPUTE;
                        vecitem[0].second = false;
                        vecitem[1].second = false;
                    }



                }
                
                else if ( object == PO_OBJ_POSE)
                {
                    

                    policeforuse = POLICIES_COMPUTE_ADJUST_KEEP;
                    num = 3;
                    vecitem.resize(3);
                    vecitem[0].first = POLICIES_COMPUTE;
                    vecitem[1].first = POLICIES_ADJUST;
                    vecitem[2].first = POLICIES_KEEP;
                    defaultpolice = POLICIES_COMPUTE;

                    bool cancompute = true;
                    bool canadjust = atdata_.AreAllImagesPoseComplete();
                    bool cankeep = canadjust;
                    if (canadjust)
                    {
                       
                        {
                            defaultpolice = POLICIES_ADJUST;
                        }
                        vecitem[0].second = true;
                        vecitem[1].second = true;
                        vecitem[2].second = true;

                    }
                    else
                    {
                        defaultpolice = POLICIES_COMPUTE;
                        vecitem[0].second = false;
                        vecitem[1].second = false;
                        vecitem[2].second = false;
                    }

                }

                else if (object == PO_OBJ_F || object == PO_OBJ_PPA ||
                    object == PO_OBJ_RDIS || object == PO_OBJ_TDIS)
                {
                    policeforuse = POLICIES_ADJUST_KEEP;
                    num = 2;
                    vecitem.resize(2);

                    vecitem[0].first = POLICIES_ADJUST;
                    vecitem[1].first = POLICIES_KEEP;
                    defaultpolice = POLICIES_ADJUST;

                    vecitem[0].second = true;
                    vecitem[1].second = true;

                }

                return std::make_tuple(defaultpolice, vecitem);

            }
           

           

      
    }
}
