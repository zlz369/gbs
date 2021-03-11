#include <gtest/gtest.h>
#include <gbs-io/fromjson.h>
#include <gbs-render/vtkcurvesrender.h>

using namespace gbs;

TEST(tests_io, json_bsc)
{

   rapidjson::Document document;
   parse_file("C:/Users/sebastien/workspace/gbslib/data/crv_bs.json",document);


   auto curve1d = bscurve_direct<double,1>(document["1d_entities"][0]);
   ASSERT_EQ(curve1d.degree(), 3);
   auto curve2d = bscurve_direct<double,2>(document["2d_entities"][0]);
   ASSERT_EQ(curve2d.poles().size(), 4);
   ASSERT_EQ(curve2d.knotsFlats().size(), 8);
   auto curvei2d= bscurve_interp_cn<double,2>(document["2d_entities"][1]);
   ASSERT_NEAR(curvei2d.value(0.5)[0],0.0,1e-6);
   ASSERT_NEAR(curvei2d.value(0.5)[1],0.5,1e-6);
   auto curve3d = bscurve_direct<double,3>(document["3d_entities"][0]);
   ASSERT_NEAR(curve3d.poles()[2][2],1.0,1e-6);


}

TEST(tests_io, meridian_channel)
{
   std::array<double, 3> col_crv = {1., 0., 0.};
   float line_width = 2.f;
   auto f_dspc = [line_width,&col_crv](const auto &crv_l) {
      std::vector<gbs::crv_dsp<double, 2, false>> crv_dsp;
      for (auto &crv : crv_l)
      {
         crv_dsp.push_back(
            gbs::crv_dsp<double, 2, false>{
               .c = &(crv),
               .col_crv = col_crv,
               .poles_on = false,
               .line_width=line_width,
            }
         );
      }
      return crv_dsp;
   };

   rapidjson::Document document;
   parse_file("D:/Projets/Alpinovx/Retrofit_Akira/python/test_channel_solve_cax.json",document);

   std::list<gbs::BSCurve2d_d> hub_curves;
   std::list<gbs::BSCurve2d_d> shr_curves;
   std::list<gbs::BSCurve2d_d> ml_curves;
   for(auto &val : document["hub_curves"].GetArray())
   {
      hub_curves.push_back( gbs::make_bscurve<double,2>(val) );
   }

   for(auto &val : document["shr_curves"].GetArray())
   {
      shr_curves.push_back( gbs::make_bscurve<double,2>(val) );
   }

   for(auto &val : document["mean_lines"].GetArray())
   {
      ml_curves.push_back( gbs::make_bscurve<double,2>(val) );
   }

   auto hub_pnts = gbs::make_point_vec<double,2>(document["hub_corner_points"]);
   auto shr_pnts = gbs::make_point_vec<double,2>(document["shr_corner_points"]);


   std::list<gbs::BSCurve2d_d> crv_l;
   crv_l.insert(crv_l.end(),hub_curves.begin(),hub_curves.end());
   crv_l.insert(crv_l.end(),shr_curves.begin(),shr_curves.end());

      
   std::list<gbs::BSCurve2d_d> crv_m;
   for(auto i = 0 ; i < hub_pnts.size() ; i++)
   {
      gbs::points_vector_2d_d pts_(2);
      pts_[0] = hub_pnts[i];
      pts_[1] = shr_pnts[i];
      crv_m.push_back(gbs::interpolate(pts_,1,gbs::KnotsCalcMode::CHORD_LENGTH));
   }

   auto crv_dsp = f_dspc(crv_l);
   col_crv = {0., 1., 0.};
   auto crv_dsp_ml = f_dspc(ml_curves);
   col_crv = {0., 0., 0.};
   line_width = 4.f;
   gbs::plot(
      crv_dsp,
      crv_dsp_ml,
      f_dspc(crv_m)
   );
}

template<typename T,size_t P>
auto build_tfi_blend_function_with_derivatives(const std::vector<T> &ksi_i) -> std::vector<std::array<gbs::BSCurve<T,1>,P>>
{
   auto n_ksi_i = ksi_i.size();
   std::vector<gbs::BSCurve<double,1>> alpha_i;

   for(int i = 0 ; i < n_ksi_i; i++)
   {
      gbs::points_vector<double,1> dji{n_ksi_i,{0.}};
      dji[i] = {1.};
      alpha_i.push_back(gbs::interpolate(dji,ksi_i,2));
   }
   return alpha_i;
}

template<typename T>
auto build_tfi_blend_function(const std::vector<T> &ksi_i) -> std::vector<gbs::BSCurve<T,1>>
{
   auto n_ksi_i = ksi_i.size();
   std::vector<gbs::BSCurve<double, 1>> alpha_i;

   for(int i = 0 ; i < n_ksi_i; i++)
   {
      gbs::points_vector<double,1> dji{n_ksi_i,{0.}};
      dji[i] = {1.};
      alpha_i.push_back(gbs::interpolate(dji,ksi_i,2));
   }
   return alpha_i;
}

auto build_channel_curves(std::vector<gbs::BSCurve2d_d> &crv_m, std::vector<gbs::BSCurve2d_d> &crv_l, std::vector<double> &u_m, std::vector<double> &u_l )
{
   rapidjson::Document document;
   parse_file("D:/Projets/Alpinovx/Retrofit_Akira/python/test_channel_solve_cax.json",document);

   auto ml_crv =  gbs::make_bscurve<double,2>(document["mean_lines"].GetArray()[0]);
   ml_crv.changeBounds(0.,1.);

   ASSERT_TRUE(document["hub_curves"].GetArray()[0].HasMember("constrains"));
   ASSERT_TRUE(document["shr_curves"].GetArray()[0].HasMember("constrains"));
   auto hub_cstr = make_constrains_vec<double, 2, 2>(document["hub_curves"].GetArray()[0]["constrains"]);
   auto shr_cstr = make_constrains_vec<double, 2, 2>(document["shr_curves"].GetArray()[0]["constrains"]);

   u_l = gbs::knots_and_mults(ml_crv.knotsFlats()).first;
   auto hub_crv = gbs::interpolate<double, 2,2>(hub_cstr, u_l);
   auto shr_crv = gbs::interpolate<double, 2,2>(shr_cstr, u_l);


   crv_m = {hub_crv, ml_crv, shr_crv};

   auto hub_pnts = gbs::make_point_vec<double,2>(document["hub_corner_points"]);
   auto shr_pnts = gbs::make_point_vec<double,2>(document["shr_corner_points"]);
   auto  ml_pnts =  gbs::make_point_vec<double,2>(document["mean_lines"].GetArray()[0]["constrains"].GetArray()[0]);
   
   u_m = {0.,0.5,1.};
   for(auto i = 0 ; i < hub_pnts.size() ; i++)
   {
      gbs::points_vector_2d_d pts_(3);
      pts_[0] = hub_pnts[i];
      pts_[1] =  ml_pnts[i];
      pts_[2] = shr_pnts[i];
      crv_l.push_back(gbs::interpolate(pts_,u_m,1));
   }
   
}

auto f_dspc(const std::vector<gbs::BSCurve2d_d> &crv_l,std::array<double, 3> col_crv = {1., 0., 0.},float line_width = 2.f) {
   std::vector<gbs::crv_dsp<double, 2, false>> crv_dsp;
   for (auto &crv : crv_l)
   {
      crv_dsp.push_back(
         gbs::crv_dsp<double, 2, false>{
            .c = &(crv),
            .col_crv = col_crv,
            .poles_on = false,
            .line_width=line_width,
         }
      );
   }
   return crv_dsp;
};

TEST(tests_io, meridian_channel_msh)
{
   std::vector<gbs::BSCurve2d_d> crv_m; 
   std::vector<gbs::BSCurve2d_d> crv_l;
   std::vector<double> u_l;
   std::vector<double> u_m;
   build_channel_curves(crv_m,crv_l,u_m,u_l);
   //Buildind of blend functions
   auto alpha_i = build_tfi_blend_function(u_l);
   auto beta_j  = build_tfi_blend_function(u_m);
   auto n_ksi = u_l.size();
   auto n_eth = u_m.size();

   auto X1 = [&](auto ksi, double eth)
   {
      auto X1_ = std::array<double,2>{0.,0.};
      for(auto i = 0 ; i < n_ksi; i++)
      {
         X1_ += alpha_i[i](ksi)[0] * crv_l[i].value(eth);
      }
      return X1_;
   };

   auto X2 = [&](auto ksi, double eth)
   {
      auto X2_ = X1(ksi,eth);
      for(auto j = 0 ; j < n_eth; j++)
      {
         X2_ += beta_j[j](eth)[0] * (crv_m[j].value(ksi) - X1(ksi, u_m[j]) );
      }
      return X2_;
   };

   gbs::points_vector<double,2> pts;
   for(auto j =0 ; j < 50 ;j++)
   {
      // auto j = 0;
      for (auto i = 0; i < 21; i++)
      {
         pts.push_back(
             X2(j / 49., i / 20.));
      }
   }

   auto crv_l_dsp = f_dspc(crv_m);
   auto crv_m_dsp = f_dspc(crv_l,{0., 1., 0.},4.f);

   gbs::plot(
      crv_l_dsp,
      crv_m_dsp,
      pts
   );
}



TEST(tests_io, meridian_channel_msh2)
{

   std::array<double, 3> col_crv = {1., 0., 0.};
   float line_width = 2.f;
   auto f_dspc = [line_width,&col_crv](const auto &crv_l) {
      std::vector<gbs::crv_dsp<double, 2, false>> crv_dsp;
      for (auto &crv : crv_l)
      {
         crv_dsp.push_back(
            gbs::crv_dsp<double, 2, false>{
               .c = &(crv),
               .col_crv = col_crv,
               .poles_on = false,
               .line_width=line_width,
            }
         );
      }
      return crv_dsp;
   };

   rapidjson::Document document;
   parse_file("D:/Projets/Alpinovx/Retrofit_Akira/python/test_channel_solve_cax.json",document);

   auto ml_crv =  gbs::make_bscurve<double,2>(document["mean_lines"].GetArray()[0]);
   ml_crv.changeBounds(0.,1.);

   ASSERT_TRUE(document["hub_curves"].GetArray()[0].HasMember("constrains"));
   ASSERT_TRUE(document["shr_curves"].GetArray()[0].HasMember("constrains"));
   auto hub_cstr = make_constrains_vec<double, 2, 2>(document["hub_curves"].GetArray()[0]["constrains"]);
   auto shr_cstr = make_constrains_vec<double, 2, 2>(document["shr_curves"].GetArray()[0]["constrains"]);

   auto u_l = gbs::knots_and_mults(ml_crv.knotsFlats()).first;
   auto hub_crv = gbs::interpolate<double, 2,2>(hub_cstr, u_l);
   auto shr_crv = gbs::interpolate<double, 2,2>(shr_cstr, u_l);


   std::vector<gbs::BSCurve2d_d> crv_m{hub_crv, ml_crv, shr_crv};

   auto hub_pnts = gbs::make_point_vec<double,2>(document["hub_corner_points"]);
   auto shr_pnts = gbs::make_point_vec<double,2>(document["shr_corner_points"]);
   auto  ml_pnts =  gbs::make_point_vec<double,2>(document["mean_lines"].GetArray()[0]["constrains"].GetArray()[0]);
   std::vector<gbs::BSCurve2d_d> crv_l;
   std::vector<double> u_m = {0.,0.5,1.};
   for(auto i = 0 ; i < hub_pnts.size() ; i++)
   {
      gbs::points_vector_2d_d pts_(3);
      pts_[0] = hub_pnts[i];
      pts_[1] =  ml_pnts[i];
      pts_[2] = shr_pnts[i];
      crv_l.push_back(gbs::interpolate(pts_,u_m,1));
   }
   //Buildind of blend functions
   auto n_ksi = u_l.size();
   auto n_eth = u_m.size();
   
   const auto P = 2;
   const auto Q = 1;
   std::vector<std::array<gbs::BSCurve<double,1>,P>> alpha_i;
   std::vector<std::array<gbs::BSCurve<double,1>,Q>> beta_j;

   for(int i = 0 ; i < n_ksi; i++)
   {
      alpha_i.push_back(std::array<gbs::BSCurve<double,1>,P>{});
      for(auto n = 0 ; n < P ; n++ )
      {
         std::vector<gbs::constrType<double, 1, P>> dji{n_ksi,{0.}};
         dji[i][n] = {1.};
         alpha_i.back()[n]=gbs::interpolate(dji,u_l);
         // alpha_i.back()[n]=gbs::interpolate(dji,u_l);
      }
      // By construction corner points are at knots values
      // alpha_i.push_back(gbs::interpolate(dji,u_l,2));
   }
   for(int j = 0 ; j < n_eth; j++)
   {
      beta_j.push_back(std::array<gbs::BSCurve<double,1>,Q>{});
      for(auto m = 0 ; m < Q ; m++ )
      {
         std::vector<gbs::constrType<double, 1, Q>> dji{n_eth,{0.}};
         dji[j][m] = {1.};
         beta_j.back()[m]=gbs::interpolate(dji,u_m);
      }
      // gbs::points_vector<double,1> dji{n_eth,{0.}};
      // dji[j] = {1.};
      // beta_j.push_back(gbs::interpolate(dji,u_m,2));
   }


   auto X1 = [&](auto ksi, double eth)
   {
      auto X1_ = std::array<double,2>{0.,0.};
      for(auto i = 0 ; i < n_ksi; i++)
      {
         // X1_ += alpha_i[i](ksi)[0] * crv_l[i].value(eth);
         for(auto n = 0 ; n < P ; n++)
         {
            X1_ += alpha_i[i][n](ksi)[0] * crv_l[i].value(eth,n);
         }
      }
      return X1_;
   };

   auto X2 = [&](auto ksi, double eth)
   {
      auto X2_ = X1(ksi,eth);
      for(auto j = 0 ; j < n_eth; j++)
      {
         // X2_ += beta_j[j](eth)[0] * (crv_m[j].value(ksi) - X1(ksi, u_l[j]) );
         for(auto m = 0 ; m < Q ; m++)
         {
            X2_ += beta_j[j][m](eth)[0] * crv_m[j].value(ksi,m);
         }
         X2_ += -beta_j[j][0](eth)[0] * X1(ksi, u_m[j]); // works only for the specific 2d planar case
      }
      return X2_;
   };

   gbs::points_vector<double,2> pts;
   for(auto j =0 ; j < 50 ;j++)
   {
      // auto j = 0;
      for (auto i = 0; i < 21; i++)
      {
         pts.push_back(
             X2(j / 49., i / 20.));
      }
   }

   auto crv_l_dsp =       f_dspc(crv_m);
   col_crv = {0., 1., 0.};
   line_width = 4.f;
   auto crv_m_dsp =       f_dspc(crv_l);

   gbs::plot(
      crv_l_dsp,
      crv_m_dsp,
      pts
   );
}


TEST(tests_io, meridian_channel_msh3)
{

   std::array<double, 3> col_crv = {1., 0., 0.};
   float line_width = 2.f;
   auto f_dspc = [line_width,&col_crv](const auto &crv_l) {
      std::vector<gbs::crv_dsp<double, 2, false>> crv_dsp;
      for (auto &crv : crv_l)
      {
         crv_dsp.push_back(
            gbs::crv_dsp<double, 2, false>{
               .c = &(crv),
               .col_crv = col_crv,
               .poles_on = false,
               .line_width=line_width,
            }
         );
      }
      return crv_dsp;
   };

   rapidjson::Document document;
   parse_file("D:/Projets/Alpinovx/Retrofit_Akira/python/test_channel_solve_cax.json",document);

   auto ml_crv =  gbs::make_bscurve<double,2>(document["mean_lines"].GetArray()[0]);
   ml_crv.changeBounds(0.,1.);

   ASSERT_TRUE(document["hub_curves"].GetArray()[0].HasMember("constrains"));
   ASSERT_TRUE(document["shr_curves"].GetArray()[0].HasMember("constrains"));
   auto hub_cstr = make_constrains_vec<double, 2, 2>(document["hub_curves"].GetArray()[0]["constrains"]);
   auto shr_cstr = make_constrains_vec<double, 2, 2>(document["shr_curves"].GetArray()[0]["constrains"]);

   auto u_l = gbs::knots_and_mults(ml_crv.knotsFlats()).first;
   auto hub_crv = gbs::interpolate<double, 2,2>(hub_cstr, u_l);
   auto shr_crv = gbs::interpolate<double, 2,2>(shr_cstr, u_l);


   std::vector<gbs::BSCurve2d_d> crv_m{hub_crv, ml_crv, shr_crv};

   auto hub_pnts = gbs::make_point_vec<double,2>(document["hub_corner_points"]);
   auto shr_pnts = gbs::make_point_vec<double,2>(document["shr_corner_points"]);
   auto  ml_pnts =  gbs::make_point_vec<double,2>(document["mean_lines"].GetArray()[0]["constrains"].GetArray()[0]);
   std::vector<gbs::BSCurve2d_d> crv_l;
   std::vector<double> u_m = {0.,0.5,1.};
   for(auto i = 0 ; i < hub_pnts.size() ; i++)
   {
      gbs::points_vector_2d_d pts_(3);
      pts_[0] = hub_pnts[i];
      pts_[1] =  ml_pnts[i];
      pts_[2] = shr_pnts[i];
      crv_l.push_back(gbs::interpolate(pts_,u_m,1));
   }
   //Buildind of blend functions
   auto n_ksi = u_l.size();
   auto n_eth = u_m.size();
   
   const auto P = 2;
   const auto Q = 1;
   std::vector<gbs::BSCurve<double,1>> alpha_i;
   std::vector<gbs::BSCurve<double,1>> beta_j;

   for(int i = 0 ; i < n_ksi; i++)
   {

      std::vector<gbs::constrType<double, 1, P>> dji{n_ksi,{0.}};
      dji[i][0] = {1.};
      alpha_i.push_back(gbs::interpolate(dji,u_l));
      // By construction corner points are at knots values
      // alpha_i.push_back(gbs::interpolate(dji,u_l,2));
   }
   for(int j = 0 ; j < n_eth; j++)
   {
      std::vector<gbs::constrType<double, 1, 2>> dji{n_eth,{0.}};
      dji[j][0] = {1.};
      beta_j.push_back(gbs::interpolate(dji,u_m));
   }


   auto X1 = [&](auto ksi, double eth)
   {
      auto X1_ = std::array<double,2>{0.,0.};
      for(auto i = 0 ; i < n_ksi; i++)
      {
         X1_ += alpha_i[i](ksi)[0] * crv_l[i].value(eth);
      }
      return X1_;
   };

   auto X2 = [&](auto ksi, double eth)
   {
      auto X2_ = X1(ksi,eth);
      for(auto j = 0 ; j < n_eth; j++)
      {
         X2_ += beta_j[j](eth)[0] * (crv_m[j].value(ksi) - X1(ksi, u_m[j]) );
      }
      return X2_;
   };

   gbs::points_vector<double,2> pts;
   for(auto j =0 ; j < 50 ;j++)
   {
      // auto j = 0;
      for (auto i = 0; i < 21; i++)
      {
         pts.push_back(
             X2(j / 49., i / 20.));
      }
   }

   auto crv_l_dsp =       f_dspc(crv_m);
   col_crv = {0., 1., 0.};
   line_width = 4.f;
   auto crv_m_dsp =       f_dspc(crv_l);

   gbs::plot(
      crv_l_dsp,
      crv_m_dsp,
      pts
   );
}