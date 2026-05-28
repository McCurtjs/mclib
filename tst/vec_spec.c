/*******************************************************************************
* MIT License
*
* Copyright (c) 2025 Curtis McCoy
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

#include "vec.h"

#include "cspec.h"

describe(vector_z_order_curve) {

  it("tests the z-curve in both directions") {
    size_t i = 246810;
    vec2i v = i2zcurve(i);
    expect(i2zindex(v) == i);
  }

}

describe(test_handedness) {
  vec3 result = v3zero;
  vec3 expected = v3ones;

  it("verifies the right-hand coordinate system (XxY)") {
    result = v3cross(v3x, v3y);
    expected = v3z;
  }

  it("verifies the right-hand coordinate system (YxZ)") {
    result = v3cross(v3y, v3z);
    expected = v3x;
  }

  it("verifies the right-hand coordinate system (ZxX)") {
    result = v3cross(v3z, v3x);
    expected = v3y;
  }

  it("verifies the right-hand coordinate system (YxX)") {
    result = v3cross(v3y, v3x);
    expected = v3f(0, 0, -1);
  }

  it("verifies the right-hand coordinate system (ZxY)") {
    result = v3cross(v3z, v3y);
    expected = v3f(-1, 0, 0);
  }

  it("verifies the right-hand coordinate system (XxZ)") {
    result = v3cross(v3x, v3z);
    expected = v3f(0, -1, 0);
  }

  after {
    expect(result.x to be_about(expected.x));
    expect(result.y to be_about(expected.y));
    expect(result.z to be_about(expected.z));
  }

}

test_suite(tests_vec) {
  test_group(vector_z_order_curve),
  test_group(test_handedness),
  test_suite_end
};
