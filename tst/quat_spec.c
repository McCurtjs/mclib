/*******************************************************************************
* MIT License
*
* Copyright (c) 2026 Curtis McCoy
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "quat.h"

#include "cspec.h"

describe(invariants) {
  vec3 begin = v3right;
  vec3 expected = v3zero;
  quat test = q4identity;

  it("maintains identity") {
    vec3 result = v3rotate(begin, q4identity);
    expect(result.x to be_about(begin.x));
    expect(result.y to be_about(begin.y));
    expect(result.z to be_about(begin.z));
  }

  context("matches quaternion rotation with matrix form") {

    it("does not change with identity") {
      expected = v3right;
    }

    it("rotates along the XY plane 90 degrees") {
      test = q4axang(v3z, PI / 2.f);
      expected = v3f(0.f, 1.f, 0.f);
    }

    it("rotates along the XY plane 45 degrees") {
      test = q4axang(v3z, PI / 4.f);
      expected = v3f(1.f / SQRT2, 1.f / SQRT2, 0.f);
    }

    it("rotates along the XZ plane 90 degrees") {
      test = q4axang(v3y, PI / 2.f);
      expected = v3f(0.f, 0.f, -1.f);
    }

    it("rotates along the XZ plane 45 degrees") {
      test = q4axang(v3y, PI / 4.f);
      expected = v3f(1.f / SQRT2, 0.f, -1.f / SQRT2);
    }

    it("rotates along the YZ plane 90 degrees") {
      begin = v3z;
      test = q4axang(v3x, PI / 2.f);
      expected = v3f(0.f, -1.f, 0.f);
    }

    it("rotates along the YZ plane 45 degrees") {
      begin = v3z;
      test = q4axang(v3x, PI / 4.f);
      expected = v3f(0.f, -1.f / SQRT2, 1.f / SQRT2);
    }

    after {
      vec3 result_v = v3rotate(begin, test);
      vec3 result_m = mv4mul(m4q(test), v34(begin)).xyz;

      expect(result_v.x to be_about(expected.x));
      expect(result_v.y to be_about(expected.y));
      expect(result_v.z to be_about(expected.z));
      expect(result_m.x to be_about(expected.x));
      expect(result_m.y to be_about(expected.y));
      expect(result_m.z to be_about(expected.z));
    }

  }

  it("matches quternion and matrix multiplucation") {
    quat a = q4norm(q4f(1, 2, 3, 4));
    quat b = q4norm(q4f(3,-2, 1, 0));

    mat4 result_q = m4q(q4mul(a, b));
    mat4 result_m = m4mul(m4q(a), m4q(b));

    for (int i = 0; i < m4floats; ++i) {
      expect(result_q.f[i] to be_about(result_m.f[i]));
    }
  }

  context("it correctly handles inverse identity relationships") {

    it("validates inverse invariant") {
      test = q4f(3, 2, 4, 1);
      test = q4mul(test, q4inv(test));
      expect(test.i to be_about(0));
      expect(test.j to be_about(0));
      expect(test.k to be_about(0));
      expect(test.w to be_about(1));
    }

    it("validates conjugate as inverse with unit quaternion") {
      test = q4norm(q4f(5, 1, -2, 6));
      test = q4mul(test, q4conj(test));
      expect(test.i to be_about(0));
      expect(test.j to be_about(0));
      expect(test.k to be_about(0));
      expect(test.w to be_about(1));
    }

    it("validates identity with inverse rotation") {
      test = q4norm(q4f(2, 8, -4, 3));
      begin = v3rand_dir();
      vec3 result = v3rotate(v3rotate(begin, test), q4inv(test));
      expect(result.x to be_about(begin.x));
      expect(result.y to be_about(begin.y));
      expect(result.z to be_about(begin.z));
    }

  }

}

describe(q4look) {

  it("creates a rotation looking in a certain direction") {
    quat q = v3look(v3left, v3up);
    vec3 v = v3rotate(v3front, q);
    expect(v.x to be_about(-1));
    expect(v.y to be_about(0));
    expect(v.z to be_about(0));
  }

  it("creates a rotation looking to a non-axis direction") {
    quat q = v3look(v3f(1, 2, 3), v3up);
    vec3 v = v3rotate(v3front, q);
    vec3 expected = v3norm(v3f(1, 2, 3));
    expect(v.x to be_about(expected.x));
    expect(v.y to be_about(expected.y));
    expect(v.z to be_about(expected.z));
  }

}

describe(q4m) {

  it("converts a rotational matrix to a quaternion") {
    mat3 basis = m3v(v3right, v3up, v3front);
    quat q = q4m(basis);
    mat4 M = m4q(q);

    for (int j = 0; j < 3; ++j) {
      for (int i = 0; i < 3; ++i) {
        expect(M.col[j].row[i] to be_about(basis.col[j].row[i]));
      }
    }
  }

}

test_suite(tests_quat) {
  test_group(invariants),
  test_group(q4look),
  test_group(q4m),
  test_suite_end
};
