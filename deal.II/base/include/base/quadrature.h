/*----------------------------   quadrature.h     ---------------------------*/
/*      $Id$                 */
/*      Copyright W. Bangerth, University of Heidelberg, 1998 */
#ifndef __quadrature_H
#define __quadrature_H
/*----------------------------   quadrature.h     ---------------------------*/


#include <base/point.h>
#include <vector>



/**
 * Base class for quadrature formulae in arbitrary dimensions. This class
 * stores quadrature points and weights on the unit line [0,1], unit
 * square [0,1]x[0,1], etc. This information is used together with
 * objects of the \Ref{FiniteElement} class to compute the values stored
 * in the \Ref{FEValues} objects.
 *
 * There are a number of derived classes, denoting concrete integration
 * formulae. These are named by a prefixed #Q#, the name of the formula
 * (e.g. #Gauss#) and finally the order of integration. For example,
 * #QGauss2<dim># denotes a second order Gauss integration formula in
 * any dimension. Second order means that it integrates polynomials of
 * third order exact. In general, a formula of order #n# exactly
 * integrates polynomials of order #2n-1#.
 *
 * Most integration formulae in more than one space dimension are tensor
 * products of quadrature formulae in one space dimension, or more
 * generally the tensor product of a formula in #(dim-1)# dimensions and
 * one in one dimension. There is a special constructor to generate a
 * quadrature formula from two others.
 *
 *
 * For some programs it is necessary to have a quadrature object for faces.
 * These programs fail to link if compiled for only one space dimension,
 * since there quadrature rules for faces just don't make no sense. In
 * order to allow these programs to be linked anyway, for class #Quadrature<0>#
 * all functions are provided in the #quadrature.cc# file, but they will
 * throw exceptions if actually called. The only function which is allowed
 * to be called is the constructor taking one integer, which in this case
 * ignores its parameter, and of course the destructor.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <int dim>
class Quadrature {
  public:
				     /**
				      * Number of quadrature points.
				      */
    const unsigned int n_quadrature_points;

				     /**
				      * Constructor.
				      */
    Quadrature (const unsigned int n_quadrature_points);

				     /**
				      * Build this quadrature formula as the
				      * tensor product of a formula in a
				      * dimension one less than the present and
				      * a formula in one dimension.
				      */
    Quadrature (const Quadrature<dim-1> &,
		const Quadrature<1>     &);
    
				     /**
				      * Virtual destructor needed, since someone
				      * may want to use pointers to this class
				      * though the object pointed to is a derived
				      * class.
				      */
    virtual ~Quadrature ();
    
				     /**
				      * Return the #i#th quadrature point.
				      */
    const Point<dim> & quad_point (const unsigned int i) const;

				     /**
				      * Return a reference to the whole array of
				      * quadrature points.
				      */
    const vector<Point<dim> > & get_quad_points () const;
    
				     /**
				      * Return the weight of the #i#th
				      * quadrature point.
				      */
    double weight (const unsigned int i) const;

				     /**
				      * Return a reference to the whole array
				      * of weights.
				      */
    const vector<double> & get_weights () const;
    
				     /**
				      * Exception
				      */
    DeclException2 (ExcInvalidIndex,
		    int, int,
		    << "The index " << arg1
		    << " is out of range, it should be less than " << arg2);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInternalError);
    
  protected:
				     /**
				      * List of quadrature points. To be filled
				      * by the constructors of derived classes.
				      */
    vector<Point<dim> > quadrature_points;

				     /**
				      * List of weights of the quadrature points.
				      * To be filled by the constructors of
				      * derived classes.
				      */
    vector<double>      weights;
};



/**
 * Quadrature formula constructed by iteration of another quadrature formula in
 * each direction. In more than one space dimension, the resulting quadrature
 * formula is constructed in the usual way by building the tensor product of
 * the respective iterated quadrature formula in one space dimension.
 *
 * In one space dimension, the given base formula is copied and scaled onto
 * a given number of subintervals of length #1/n_copies#. If the quadrature
 * formula uses both end points of theunit interval, then in the interior
 * of the iterated quadrature formula there would be quadrature points which
 * are used twice; we merge them into one with a weight which is the sum
 * of the weights of the left- and the rightmost quadrature point.
 *
 * Since all dimensions higher than one are built up by tensor products of
 * one dimensional and #dim-1# dimensional quadrature formulae, the
 * argument given to the constructor needs to be a quadrature formula in
 * one space dimension, rather than in #dim# dimensions.
 *
 * @author Wolfgang Bangerth 1999
 */
template <int dim>
class QIterated : public Quadrature<dim>
{
  public:
				     /**
				      * Constructor. Iterate the given
				      * quadrature formula $n_copies$ times in
				      * each direction.
				      */
    QIterated (const Quadrature<1> &base_quadrature,
	       const unsigned int   n_copies);

				     /**
				      * Exception
				      */
    DeclException0 (ExcSumOfWeightsNotOne);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInvalidQuadratureFormula);
    
  private:
				     /**
				      * check whether the given
				      * quadrature formula has quadrature
				      * points at the left and right end points
				      * of the interval
				      */
    static bool uses_both_endpoints (const Quadrature<1> &base_quadrature);
};




/**
 *  This class is a helper class to facilitate the usage of quadrature formulae
 *  on faces or subfaces of cells. It computes the locations of quadrature
 *  points on the unit cell from a quadrature object for a mannifold of
 *  one dimension less than that of the cell and the number of the face.
 *  For example, giving the Simpson rule in one dimension and using the
 *  #project_to_face# function with face number 1, the returned points will
 *  be $(1,0)$, $(1,0.5)$ and $(1,1)$. Note that faces have an orientation,
 *  so when projecting to face 3, you will get $(0,0)$, $(0,0.5)$ and $(0,1)$,
 *  which is in clockwise sense, while for face 1 the points were in
 *  counterclockwise sense.
 *
 *  For the projection to subfaces (i.e. to the children of a face of the
 *  unit cell), the same applies as above. Note the order in which the
 *  children of a face are numbered, which in two dimensions coincides
 *  with the orientation of the face.
 *  
 *  The different functions are grouped into a common class to avoid putting
 *  them into global namespace (and to make documentation easier, since
 *  presently the documentation tool can only handle classes, not global
 *  functions). However, since they have no local data, all functions are
 *  declared #static# and can be called without creating an object of this
 *  class.
 *
 *  For the 3d case, you should note that the orientation of faces is even
 *  more intricate than for two dimensions. Quadrature formulae are projected
 *  upon the faces in their standard orientation, not to the inside or outside
 *  of the hexahedron! Refer to the documentation of the #Triangulation# class
 *  for a description of the orientation of the different faces.
 */
template <int dim>
class QProjector {
  public:
				     /**
				      * Compute the quadrature points on the
				      * cell if the given quadrature formula
				      * is used on face #face_no#. For further
				      * details, see the general doc for
				      * this class.
				      */
    static void project_to_face (const Quadrature<dim-1> &quadrature,
				 const unsigned int       face_no,
				 vector<Point<dim> >     &q_points);

    				     /**
				      * Compute the quadrature points on the
				      * cell if the given quadrature formula
				      * is used on face #face_no#, subface
				      * number #subface_no#. For further
				      * details, see the general doc for
				      * this class.
				      */
    static void project_to_subface (const Quadrature<dim-1> &quadrature,
				    const unsigned int       face_no,
				    const unsigned int       subface_no,
				    vector<Point<dim> >     &q_points);

				     /**
				      * Exception
				      */
    DeclException2 (ExcInvalidIndex,
		    int, int,
		    << "The index " << arg1
		    << " is out of range, it should be less than " << arg2);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInternalError);
};





/*----------------------------   quadrature.h     ---------------------------*/
/* end of #ifndef __quadrature_H */
#endif
/*----------------------------   quadrature.h     ---------------------------*/
