#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <gbs/curves>
#include <gbs/surfaces>

namespace gbs
{

    // compatible with most iges types
    enum class entity_type {
        BSCfunction = 25,
        SurfaceOfRevolution = 120,
        BSCurve = 125,
        BSCurveRational = 126,
        BSSurface = 127,
        BSSurfaceRational = 128,
        // CurveOffset = 139,
        Curve2dOffset = 138,
        CurveOnSurface = 142
    };
    
    auto make_json(const auto &v, auto &allocator)
    {
        rapidjson::Value v_val{rapidjson::kArrayType};
        std::for_each(
            v.begin(), v.end(),
            [&](auto value){v_val.PushBack(value,allocator);}
        );
        return v_val;
    }
    // template< typename T>
    // auto make_json(const std::vector<T> &v, auto &allocator)
    // {
    //     rapidjson::Value v_val{rapidjson::kArrayType};
    //     std::for_each(
    //         v.begin(), v.end(),
    //         [&](auto value)
    //         { v_val.PushBack(value, allocator); });
    //     return v_val;
    // }

    template< typename T, size_t dim>
    auto make_json(const std::vector<std::array<T,dim>> &v, auto &allocator)
    {
        rapidjson::Value v_val{rapidjson::kArrayType};
        std::for_each(
            v.begin(), v.end(),
            [&](const auto &value){
                v_val.PushBack(make_json(value,allocator),allocator);
            }
        );
        return v_val;
    }

    template< typename T, size_t dim, size_t n>
    auto make_json(const std::array<std::array<T,dim>, n> &v, auto &allocator)
    {
        rapidjson::Value v_val{rapidjson::kArrayType};
        std::for_each(
            v.begin(), v.end(),
            [&](const auto &value){
                v_val.PushBack(make_json(value,allocator),allocator);
            }
        );
        return v_val;
    }

    template< typename T, size_t dim>
    auto make_json(const BSCurve<T,dim> &crv, auto &allocator)
    {
        rapidjson::Value crv_val;
        crv_val.SetObject();
        rapidjson::Value deg_val{crv.degree()};
        rapidjson::Value dim_val{dim};
        rapidjson::Value type_val{static_cast<int>(entity_type::BSCurve)};
        auto [knots, mults] = knots_and_mults(crv.knotsFlats());
        auto knots_val = make_json(knots,allocator);
        auto mults_val = make_json(mults,allocator);
        auto poles_val = make_json(crv.poles(),allocator);

        crv_val.AddMember( "type"   ,type_val, allocator);
        crv_val.AddMember( "degree" ,deg_val, allocator);
        crv_val.AddMember( "dim"    ,dim_val, allocator);
        crv_val.AddMember( "knots"  ,knots_val, allocator);
        crv_val.AddMember( "mults"  ,mults_val, allocator);
        crv_val.AddMember( "poles"  ,poles_val, allocator);

        return crv_val;
    }

    template< typename T, size_t dim>
    auto make_json(const BSCurveRational<T,dim> &crv, auto &allocator)
    {
        rapidjson::Value crv_val;
        crv_val.SetObject();
        rapidjson::Value deg_val{crv.degree()};
        rapidjson::Value dim_val{dim};
        rapidjson::Value type_val{static_cast<int>(entity_type::BSCurveRational)};
        auto [knots, mults] = knots_and_mults(crv.knotsFlats());
        auto knots_val = make_json(knots,allocator);
        auto mults_val = make_json(mults,allocator);
        auto poles_val   = make_json(crv.polesProjected(),allocator);
        auto weights_val = make_json(crv.weights(),allocator);

        crv_val.AddMember( "type"    ,type_val, allocator);
        crv_val.AddMember( "degree"  ,deg_val, allocator);
        crv_val.AddMember( "dim"     ,dim_val, allocator);
        crv_val.AddMember( "knots"   ,knots_val, allocator);
        crv_val.AddMember( "mults"   ,mults_val, allocator);
        crv_val.AddMember( "poles"   ,poles_val, allocator);
        crv_val.AddMember( "weights" ,weights_val, allocator);

        return crv_val;
    }

    template< typename T>
    auto make_json(const BSCfunction<T> &f, auto &allocator)
    {
        rapidjson::Value f_val;
        f_val.SetObject();
        rapidjson::Value type_val{static_cast<int>(entity_type::BSCfunction)};
        auto crv_val = make_json<T,1>(f.basisCurve(), allocator);

        f_val.AddMember( "type"    ,type_val, allocator);
        f_val.AddMember( "curve"   ,crv_val, allocator);

        return f_val;
    }

    template< typename T>
    auto make_json(const gbs::CurveOffset<T, 2,gbs::BSCfunction<T>> & offset, auto &allocator)
    {
        rapidjson::Value offset_val;
        offset_val.SetObject();
        rapidjson::Value type_val{static_cast<int>(entity_type::Curve2dOffset)};
        auto crv_val = make_json<T,2>(&offset.basisCurve(), allocator);
        auto f_val   = make_json(offset.offset(), allocator);

        offset_val.AddMember( "type"     ,type_val, allocator);
        offset_val.AddMember( "curve"    ,crv_val, allocator);
        offset_val.AddMember( "function" ,f_val, allocator);

        return offset_val;
    }

    template< typename T , size_t dim>
    auto make_json(const Curve<T,dim> *crv, auto &allocator)
    {
        if(dynamic_cast<const BSCurve<T,dim>*>(crv))
        {
            return make_json(*static_cast<const BSCurve<T,dim>*>(crv),allocator);
        }
        if(dynamic_cast<const BSCurveRational<T,dim>*>(crv))
        {
            return make_json(*static_cast<const BSCurveRational<T,dim>*>(crv),allocator);
        }
        rapidjson::Value null_val;
        return null_val;
    }


    template <typename T >
    auto make_json(const SurfaceOfRevolution<T> &srf, auto &allocator)
    {
        rapidjson::Value srf_val;
        srf_val.SetObject();
        rapidjson::Value dim_val{2};
        rapidjson::Value type_val{static_cast<int>(entity_type::SurfaceOfRevolution)};
        auto ax2_val = make_json<T,3,3>(srf.axis2(), allocator);
        auto [u1, u2, th1, th2] = srf.bounds();
        rapidjson::Value th1_val{th1};
        rapidjson::Value th2_val{th2};
        auto crv_val = make_json<T,2>(srf.basisCurve().get(), allocator);

        srf_val.AddMember( "type"    ,type_val, allocator);
        srf_val.AddMember( "dim"     ,dim_val, allocator);
        srf_val.AddMember("axis2",ax2_val, allocator);
        srf_val.AddMember("theta1",th1_val, allocator);
        srf_val.AddMember("theta2",th2_val, allocator);
        srf_val.AddMember("curve",crv_val, allocator);

        return srf_val;
    }

    template< typename T, size_t dim>
    auto make_json(const BSSurface<T,dim> &srf, auto &allocator)
    {
        rapidjson::Value srf_val;
        srf_val.SetObject();
        rapidjson::Value degU_val{srf.degreeU()};
        rapidjson::Value degV_val{srf.degreeV()};
        rapidjson::Value dim_val{dim};
        rapidjson::Value type_val{static_cast<int>(entity_type::BSSurface)};
        auto [knotsU, multsU] = knots_and_mults(srf.knotsFlatsU());
        auto [knotsV, multsV] = knots_and_mults(srf.knotsFlatsV());
        auto knotsU_val = make_json(knotsU,allocator);
        auto multsU_val = make_json(multsU,allocator);
        auto knotsV_val = make_json(knotsV,allocator);
        auto multsV_val = make_json(multsV,allocator);
        auto poles_val = make_json(srf.poles(),allocator);

        srf_val.AddMember( "type"    ,type_val, allocator);
        srf_val.AddMember( "degreeU" ,degU_val, allocator);
        srf_val.AddMember( "degreeV" ,degV_val, allocator);
        srf_val.AddMember( "dim"     ,dim_val, allocator);
        srf_val.AddMember( "knotsU"  ,knotsU_val, allocator);
        srf_val.AddMember( "multsU"  ,multsU_val, allocator);
        srf_val.AddMember( "knotsV"  ,knotsV_val, allocator);
        srf_val.AddMember( "multsV"  ,multsV_val, allocator);
        srf_val.AddMember( "poles"   ,poles_val, allocator);

        return srf_val;
    }

    template< typename T, size_t dim>
    auto make_json(const BSSurfaceRational<T,dim> &srf, auto &allocator)
    {
        rapidjson::Value srf_val;
        srf_val.SetObject();
        rapidjson::Value degU_val{srf.degreeU()};
        rapidjson::Value degV_val{srf.degreeV()};
        rapidjson::Value dim_val{dim};
        rapidjson::Value type_val{static_cast<int>(entity_type::BSSurfaceRational)};
        auto [knotsU, multsU] = knots_and_mults(srf.knotsFlatsU());
        auto [knotsV, multsV] = knots_and_mults(srf.knotsFlatsV());
        auto knotsU_val = make_json(knotsU,allocator);
        auto multsU_val = make_json(multsU,allocator);
        auto knotsV_val = make_json(knotsV,allocator);
        auto multsV_val = make_json(multsV,allocator);
        auto poles_val   = make_json(srf.polesProjected(),allocator);
        auto weights_val = make_json(srf.weights(),allocator);

        srf_val.AddMember( "type"     ,type_val, allocator);
        srf_val.AddMember( "degreeU"   ,degU_val, allocator);
        srf_val.AddMember( "degreeV"   ,degV_val, allocator);
        srf_val.AddMember( "dim"       ,dim_val, allocator);
        srf_val.AddMember( "knotsU"    ,knotsU_val, allocator);
        srf_val.AddMember( "multsU"    ,multsU_val, allocator);
        srf_val.AddMember( "knotsV"    ,knotsV_val, allocator);
        srf_val.AddMember( "multsV"    ,multsV_val, allocator);
        srf_val.AddMember( "poles"     ,poles_val, allocator);
        srf_val.AddMember( "weights"   ,weights_val, allocator);

        return srf_val;
    }

    template< typename T , size_t dim>
    auto make_json(const Surface<T,dim> *srf, auto &allocator)
    {
        if(dynamic_cast<const BSSurface<T,dim>*>(srf))
        {
            return make_json(*static_cast<const BSSurface<T,dim>*>(srf),allocator);
        }
        if(dynamic_cast<const BSSurfaceRational<T,dim>*>(srf))
        {
            return make_json(*static_cast<const BSSurfaceRational<T,dim>*>(srf),allocator);
        }
        if(dim == 3 && dynamic_cast<const SurfaceOfRevolution<T>*>(srf))
        {
            return make_json(*static_cast<const SurfaceOfRevolution<T>*>(srf),allocator);
        }
        rapidjson::Value null_val;
        return null_val;
    }

    template< typename T , size_t dim>
    auto make_json(const CurveOnSurface<T,dim> &crv, auto &allocator)
    {
        rapidjson::Value crv_val;
        crv_val.SetObject();
        rapidjson::Value dim_val{dim};
        rapidjson::Value type_val{static_cast<int>(entity_type::CurveOnSurface)};

        auto crv2d_val = make_json<T,2>(  &crv.basisCurve(),   allocator);
        auto srf_val   = make_json<T,dim>(&crv.basisSurface(), allocator);
        
        crv_val.AddMember( "type"     ,type_val,  allocator);
        crv_val.AddMember( "dim"      ,dim_val,   allocator);
        crv_val.AddMember( "curve2d"  ,crv2d_val, allocator);
        crv_val.AddMember( "surface"  ,srf_val,   allocator);

        return crv_val;
    }
}