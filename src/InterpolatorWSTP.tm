:Evaluate: BeginPackage["BicubicInterpolatorWSTP`"]

:Evaluate: CreateInterpolator::usage = "CreateInterpolator[data] creates an interpolator from 2D data."
:Evaluate: InterpolatePoint::usage = "InterpolatePoint[handle, x, y] interpolates the value at point (x,y)."
:Evaluate: DeleteInterpolator::usage = "DeleteInterpolator[handle] removes an interpolator."

:Evaluate: Begin["`Private`"]

:Begin:
:Function:       WSTPCreateInterpolator
:Pattern:        CreateInterpolator[mat_]
:Arguments:      {mat}
:ArgumentTypes:  {Manual}
:ReturnType:     Manual
:End:

:Begin:
:Function:       WSTPInterpolatePoint
:Pattern:        InterpolatePoint[x_Real, y_Real, handle_Integer]
:Arguments:      {x, y, handle}
:ArgumentTypes:  {Real, Real, Integer}
:ReturnType:     Real
:End:

:Begin:
:Function:       WSTPDeleteInterpolator
:Pattern:        DeleteInterpolator[handle_Integer]
:Arguments:      {handle}
:ArgumentTypes:  {Integer}
:ReturnType:     Manual
:End:

:Begin:
:Function:       WSTPDeleteInterpolator
:Pattern:        DeleteInterpolator[handle_Integer]
:Arguments:      {handle}
:ArgumentTypes:  {Integer}
:ReturnType:     Manual
:End:

:Begin:
:Function: WSTPCreateSimpsonIntegrator
:Pattern: CreateSimpsonIntegrator[func_, n_Integer]
:Arguments: {n, func}
:ArgumentTypes: {Integer, Manual}
:ReturnType: Integer
:End:

:Begin:
:Function: WSTPIntegrateSimpson
:Pattern: IntegrateSimpson[handle_Integer, a_Real, b_Real]
:Arguments: {handle, a, b}
:ArgumentTypes: {Integer, Real, Real}
:ReturnType: Real
:End:

:Begin:
:Function: WSTPCreateCurveIntegrator
:Pattern: CreateCurveIntegrator[interpHandle_Integer, xFunc_, yFunc_]
:Arguments: {interpHandle, xFunc, yFunc}
:ArgumentTypes: {Integer, Manual}
:ReturnType: Integer
:End:

:Evaluate: End[]
:Evaluate: EndPackage[]