# ofxCeres

OpenFrameworks addon / helpers for working with Google Ceres solver

http://ceres-solver.org/

# Notes on glm and ceres-solver

To effectively use ceres, the solver requires the derivative of your cost function with respect to the model parameters. This derivative is called the Jacobian. The Jacobian can become complicated and intensive for the programmer to compute when you have complicated mathematics going on. Ceres therefore offers an automatic differentiation feature which will automatically compute the derivative for you. In order for this to work, it must pass a special data type through your math functions in order to calculate the differentiation, therefore your maths functions must be templated.

Here we exploit the feature of glm whereby all major maths functions are templated. However, there are some caveats and problem cases. Generally these issues come out as compile errors. Here we explain some strategies to avoid compile errors:


1. ceres doesn't play nice with swizzle, unions (e.g. x,y,z or r,g,b) and some of the other features of glm, therefore I suggest that you set `GLM_FORCE_UNRESTRICTED_GENTYPE` and `GLM_FORCE_XYZW_ONLY` project-wide. `ofxCeres.props` already includes this define.
2. Some of the maths functions of glm aren't working well. You can find some replacement functions in the `ofxCeres::VectorMath` namespace which should work as expected. Check [https://github.com/g-truc/glm/issues/973] for notes on why this is.

## Fudge to `openFrameworks\libs\glm\include\glm\detail\glmtype_quat.hpp`

We previously would change lines from 42-59 to:

```c++
#		if GLM_LANG & GLM_LANG_CXXMS_FLAG
#			if GLM_CONFIG_XYZW_ONLY
#				ifdef GLM_FORCE_QUAT_DATA_WXYZ
		struct { T w, x, y, z; };
#				else
		struct { T x, y, z, w; };
#				endif
#			else
		union
		{
#				ifdef GLM_FORCE_QUAT_DATA_WXYZ
			struct { T w, x, y, z; };
#				else
			struct { T x, y, z, w; };
#				endif

			typename detail::storage<4, T, detail::is_aligned<Q>::value>::type data;
		};
#			endif
#		else
#			ifdef GLM_FORCE_QUAT_DATA_WXYZ
				T w, x, y, z;
#			else
				T x, y, z, w;
#			endif
#		endif
```

You can find a version of the corrected file in `docs/type_quat.hpp` for GLM 0.9.9.7

This is because the default constructor will be confused with the union working on Jet types

We don't actually do this any more (it seems that simply having the defines set in the project settings is enough).

Meanwhile : watch out for include orders if you get errors about quaternions. Try to include ofxCeres LATER (that worked last time I needed to fix it).

# General notes on usage

* ceres.dll will be copied to your output folder. If you switch between Debug and Release, you might have the wrong version there. Re-build ofxCeresLib to fix.


# Building ceres-solver

For Visual Studio, please use [https://github.com/elliotwoods/ceres-windows/] branch `v2` which is configured for:
Note that the submodules seem broken (and we already spent a tonne of time on recompiling ceres for windows). A working building folder can be found at https://www.dropbox.com/s/czuo6zpktsjrbuw/ceres-windows%2020211029.zip?dl=0

* Use CXSPARSE for sparse maths library
* v2.0 of ceres-solver
* ceres built as shared library
* No tests, benchmarks, samples, etc (library only)
* glog (not miniglog)
* CXSPARSE_VERSION set manually
* /bigobj for x64 builds
* add `int FLAGS_v = 2;` to line 45 of `wall_time.cc` otherwise it prints out (+performs?) extra 5 lines of profile info at every iteration
	* Note : this is the only change within the `ceres-solver` repo itself. We don't have/use our own fork for that

You need to:

1. Clone recursively
2. Build Debug/Release x Win32/x64


# Install on macos.
Currenlty the master branch builds on macos M1.
branch [macosIntel](https://github.com/roymacdonald/ofxceres/tree/macosIntel) contains the libraries to compile on macos (min. 10.15) running on an intel processor.

It requires you to use the an openFrameworks nightly build older than of_v20220520_osx_nightly.zip or the current github master branch.

To get the nightly builds go to https://openframeworks.cc/download/ and scroll down to the end of the page.

This branch as well needs my branches for the following addons:

* https://github.com/roymacdonald/ofxAssets
* https://github.com/roymacdonald/ofxTextInputField
* https://github.com/roymacdonald/ofxPlugin
* https://github.com/roymacdonald/ofxGrabCam
* https://github.com/roymacdonald/ofxCvGui
