# ofxCeres

OpenFrameworks addon / helpers for working with Google Ceres solver

http://ceres-solver.org/

# Notes on glm and ceres-solver

To effectively use ceres, the solver requires the derivative of your cost function with respect to the model parameters. This derivative is called the Jacobian. The Jacobian can become complicated and intensive for the programmer to compute when you have complicated mathematics going on. Ceres therefore offers an automatic differentiation feature which will automatically compute the derivative for you. In order for this to work, it must pass a special data type through your math functions in order to calculate the differentiation, therefore your maths functions must be templated.

Here we exploit the feature of glm whereby all major maths functions are templated. However, there are some caveats and problem cases. Generally these issues come out as compile errors. Here we explain some strategies to avoid compile errors:


1. ceres doesn't play nice with swizzle and some of the other features of glm, therefore I suggest that you set `GLM_FORCE_UNRESTRICTED_GENTYPE` project-wide. `ofxCeres.props` already includes this define.
2. Some of the maths functions of glm aren't working well. You can find some replacement functions in the `ofxCeres::VectorMath` namespace which should work as expected. Check [https://github.com/g-truc/glm/issues/973] for notes on why this is.