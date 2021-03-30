# ofxCeres

OpenFrameworks addon / helpers for working with Google Ceres solver

http://ceres-solver.org/

# Notes on glm and ceres-solver

To effectively use ceres, the solver requires the derivative of your cost function with respect to the model parameters. This derivative is called the Jacobian. The Jacobian can become complicated and intensive for the programmer to compute when you have complicated mathematics going on. Ceres therefore offers an automatic differentiation feature which will automatically compute the derivative for you. In order for this to work, it must pass a special data type through your math functions in order to calculate the differentiation, therefore your maths functions must be templated.

Here we exploit the feature of glm whereby all major maths functions are templated. However, there are some caveats and problem cases. Generally these issues come out as compile errors. Here we explain some strategies to avoid compile errors:


1. ceres doesn't play nice with swizzle, unions (e.g. x,y,z or r,g,b) and some of the other features of glm, therefore I suggest that you set `GLM_FORCE_UNRESTRICTED_GENTYPE` and `GLM_FORCE_XYZW_ONLY` project-wide. `ofxCeres.props` already includes this define.
2. Some of the maths functions of glm aren't working well. You can find some replacement functions in the `ofxCeres::VectorMath` namespace which should work as expected. Check [https://github.com/g-truc/glm/issues/973] for notes on why this is.

## Fudge to `type_quat.hpp`

We change lines from 42 to:

```c++
#		if GLM_LANG & GLM_LANG_CXXMS_FLAG
	#		if GLM_CONFIG_XYZW_ONLY
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
#		endif
```

This is because the default constructor will be confused with the union working on Jet types

# General notes on usage

* ceres.dll will be copied to your output folder. If you switch between Debug and Release, you might have the wrong version there. Re-build ofxCeresLib to fix


# Building ceres-solver

For Visual Studio, we rely on [https://github.com/tbennun/ceres-windows/].

