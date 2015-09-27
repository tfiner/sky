#include <glm/glm.hpp>

#include "Matrix.h"

#include <cassert>
#include <random>


void QuatToQuat() {
   auto q1 = CQuaternion(1, 2, 4, 8);
   auto q2 = QuatToGlm(q1);

   assert(q1.x == q2[0]);
   assert(q1.y == q2[1]);
   assert(q1.z == q2[2]);
   assert(q1.w == q2[3]);

   auto q3 = GlmToQuat(q2);
   assert(q1.x == q3.x);
   assert(q1.y == q3.y);
   assert(q1.z == q3.z);
   assert(q1.w == q3.w);
}

void MatToMat() {
   auto prng = std::default_random_engine(419);
   auto dist = std::uniform_real_distribution<float>(0.0f, 1.0f);

   auto m1 = glm::mat4(1.0);
   auto p1 = glm::value_ptr(m1);
   for(auto i = 0; i < 16; ++i)
      p1[i] = dist(prng);

   auto m2 = GlmToMat(m1);


   const auto p2 = &m2.f1[0];

   for(auto i = 0; i < 16; ++i)
      assert(p1[i] == p2[i]);

   auto m3 = MatToGlm(m2);
   const auto p3 = glm::value_ptr(m3);

   for(auto i = 0; i < 16; ++i)
      assert(p1[i] == p3[i]);

   {
      auto m4 = m2;
      m4.Transpose();
      const auto p4 = &m4.f1[0];

      auto m5 = glm::transpose(m1);
      const auto p5 = glm::value_ptr(m5);

      for(auto i = 0; i < 16; ++i)
         assert(p4[i] == p5[i]);
   }

}


int main(int argc, char** argv) {
   QuatToQuat();
   MatToMat();
   return 0;
}

