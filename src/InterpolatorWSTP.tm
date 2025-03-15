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
:Pattern:        InterpolatePoint[link_Integer]
:Arguments:      {link}
:ArgumentTypes:  {Integer}
:ReturnType:     Integer
:End:

:Begin:
:Function:       WSTPDeleteInterpolator
:Pattern:        DeleteInterpolator[link_Integer]
:Arguments:      {link}
:ArgumentTypes:  {Integer}
:ReturnType:     Integer
:End:

:Evaluate: End[]
:Evaluate: EndPackage[]