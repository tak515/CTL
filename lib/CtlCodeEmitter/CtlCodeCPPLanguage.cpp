///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
// 
// A world-wide, royalty-free, non-exclusive right to distribute, copy,
// modify, create derivatives, and use, in source and binary forms, is
// hereby granted, subject to acceptance of this license. Performance of
// any of the aforementioned acts indicates acceptance to be bound by the
// following terms and conditions:
// 
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the Disclaimer of Warranty.
// 
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the Disclaimer of Warranty
//     in the documentation and/or other materials provided with the
//     distribution.
// 
//   * Nothing in this license shall be deemed to grant any rights to
//     trademarks, copyrights, patents, trade secrets or any other
//     intellectual property of A.M.P.A.S. or any contributors, except
//     as expressly stated herein, and neither the name of A.M.P.A.S.
//     nor of any other contributors to this software, may be used to
//     endorse or promote products derived from this software without
//     specific prior written permission of A.M.P.A.S. or contributor,
//     as appropriate.
// 
// This license shall be governed by the laws of the State of California,
// and subject to the jurisdiction of the courts therein.
// 
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO
// EVENT SHALL A.M.P.A.S., ANY CONTRIBUTORS OR DISTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#include "CtlCodeCPPLanguage.h"
#include "CtlCodeLContext.h"
#include "CtlCodeType.h"
#include "CtlCodeAddr.h"
#include "CtlCodeModule.h"
#include <iomanip>
#include <limits>
#include <half.h>
#include <deque>
#include <ctype.h>
#include <stdexcept>
#include <ImfVersion.h>

#ifdef OPENEXR_IMF_NAMESPACE
# include <ImathNamespace.h>
#else
# define IMATH_NAMESPACE Imath
#endif

#define STRINGIZE(X) #X
#define NAMESPACE(N, T) STRINGIZE(N) "::" T

namespace Ctl
{

CPPGenerator::CPPGenerator( bool cpp11 )
		: LanguageGenerator(),
		  myCPP11Mode( cpp11 ),
		  myInElse( 0 ),
		  myInModuleInit( 0 ), myInFunction( 0 ),
		  myDoForwardDecl( 0 ),
		  myCurInitType( NONE ), myCurModule( NULL )
{
}


////////////////////////////////////////


CPPGenerator::~CPPGenerator( void )
{
}


////////////////////////////////////////


std::string
CPPGenerator::getHeaderCode( void )
{
	return myHeaderStream.str();
}


////////////////////////////////////////


std::string
CPPGenerator::getCode( void )
{
	return myCodeStream.str();
}


////////////////////////////////////////


std::string
CPPGenerator::stdLibraryAndSetup( void )
{
	std::stringstream libSetupB;
	std::string precType = getPrecisionType();
	std::string precSuffix = getPrecisionFunctionSuffix();
	std::string precVec3 = getVectorType( 3 );
	std::string precVec4 = getVectorType( 4 );
	std::string precMat33 = getMatrixType( 3 );
	std::string precMat44 = getMatrixType( 4 );
	
	libSetupB <<
		"// C++ code automatically generated\n\n"
		"#include <ImathVec.h>\n"
		"#include <ImathMatrix.h>\n"
		"#include <ImathFun.h>\n"
		"#include <half.h>\n"
		"#include <float.h>\n"
		"#include <math.h>\n"
		"#include <limits.h>\n"
		"#include <iostream>\n"
		"#include <stdexcept>\n"
		"#include <limits>\n"
		"#include <vector>\n"
		"#include <CtlLookupTable.h>\n"
		"\n"
		"using namespace Ctl;\n"
		"\n"
		"namespace _ctlcc_ {\n"
		"\n"
		"struct Chromaticities { " << NAMESPACE( IMATH_NAMESPACE, "V2f" ) << " red; " << NAMESPACE( IMATH_NAMESPACE, "V2f" ) << " green; " << NAMESPACE( IMATH_NAMESPACE, "V2f" ) << " blue; " << NAMESPACE( IMATH_NAMESPACE, "V2f" ) << " white; };\n"
		"struct Box2i { " << NAMESPACE( IMATH_NAMESPACE, "V2i" ) << " min; " << NAMESPACE( IMATH_NAMESPACE, "V2i" ) << " max; };\n"
		"struct Box2f { " << NAMESPACE( IMATH_NAMESPACE, "V2f" ) << " min; " << NAMESPACE( IMATH_NAMESPACE, "V2f" ) << " max; };\n\n"
		"static inline void assert( bool v ) { if (!v) throw std::logic_error( \"Assertion failure\" ); }\n"
		"\n"
		"static inline void print_bool( bool v ) { std::cout << (v ? \"true\" : \"false\"); }\n"
		"static inline void print_int( int v ) { std::cout << v; }\n"
		"static inline void print_unsigned_int( unsigned int v ) { std::cout << v; }\n"
		"static inline void print_half( half v ) { std::cout << v; }\n"
		"static inline void print_float( " << precType << " v ) { std::cout << v; }\n"
		"static inline void print_string( const std::string &v ) { std::cout << v; }\n"
		"static inline void print_string( const char *v ) { std::cout << v; }\n"
		"\n"
		"static inline bool isfinite_f( " << precType << " v ) { return isfinite( v ); }\n"
		"static inline bool isnormal_f( " << precType << " v ) { return isnormal( v ); }\n"
		"static inline bool isnan_f( " << precType << " v ) { return isnan( v ); }\n"
		"static inline bool isinf_f( " << precType << " v ) { return isinf( v ) != 0; }\n"
		"static inline bool isfinite_h( half v ) { return v.isFinite(); }\n"
		"static inline bool isnormal_h( half v ) { return v.isNormalized(); }\n"
		"static inline bool isnan_h( half v ) { return v.isNan(); }\n"
		"static inline bool isinf_h( half v ) { return v.isInfinity() != 0; }\n"
		"\n"
		"#define FLT_POS_INF std::numeric_limits<" << precType << ">::infinity()\n"
		"#define FLT_NEG_INF (-std::numeric_limits<" << precType << ">::infinity())\n"
		"#define FLT_NAN (-std::numeric_limits<" << precType << ">::quiet_NaN())\n"
		"#define HALF_POS_INF half::posInf()\n"
		"#define HALF_NEG_INF half::negInf()\n"
		"#define HALF_NAN half::qNan()\n"
		"\n"
		"static inline " << precType << " acos( " << precType << " v ) { return acos" << precSuffix << "( v ); }\n"
		"static inline " << precType << " asin( " << precType << " v ) { return asin" << precSuffix << "( v ); }\n"
		"static inline " << precType << " atan( " << precType << " v ) { return atan" << precSuffix << "( v ); }\n"
		"static inline " << precType << " atan2( " << precType << " y, " << precType << " x ) { return atan2" << precSuffix << "( y, x ); }\n"
		"static inline " << precType << " cos( " << precType << " v ) { return cos" << precSuffix << "( v ); }\n"
		"static inline " << precType << " sin( " << precType << " v ) { return sin" << precSuffix << "( v ); }\n"
		"static inline " << precType << " tan( " << precType << " v ) { return tan" << precSuffix << "( v ); }\n"
		"static inline " << precType << " cosh( " << precType << " v ) { return cosh" << precSuffix << "( v ); }\n"
		"static inline " << precType << " sinh( " << precType << " v ) { return sinh" << precSuffix << "( v ); }\n"
		"static inline " << precType << " tanh( " << precType << " v ) { return tanh" << precSuffix << "( v ); }\n"
		"static inline " << precType << " exp( " << precType << " v ) { return exp" << precSuffix << "( v ); }\n"
		"static inline " << precType << " log( " << precType << " v ) { return log" << precSuffix << "( v ); }\n"
		"static inline " << precType << " log10( " << precType << " v ) { return log10" << precSuffix << "( v ); }\n"
		"static inline " << precType << " pow( " << precType << " x, " << precType << " y ) { return pow" << precSuffix << "( x, y ); }\n"
		"static inline " << precType << " pow10( " << precType << " y ) { return pow" << precSuffix << "( 10.0, y ); }\n"
		"static inline " << precType << " sqrt( " << precType << " v ) { return sqrt" << precSuffix << "( v ); }\n"
		"static inline " << precType << " fabs( " << precType << " v ) { return fabs" << precSuffix << "( v ); }\n"
		"static inline " << precType << " floor( " << precType << " v ) { return floor" << precSuffix << "( v ); }\n"
		"static inline " << precType << " fmod( " << precType << " x, " << precType << " y ) { return fmod" << precSuffix << "( x, y ); }\n"
		"static inline " << precType << " hypot( " << precType << " x, " << precType << " y ) { return hypot" << precSuffix << "( x, y ); }\n"
		"\n"
		"static inline half exp_h( " << precType << " v ) { return half( exp( v ) ); }\n"
		"static inline " << precType << " log_h( half v ) { return log( float( v ) ); }\n"
		"static inline " << precType << " log10_h( half v ) { return log10( float( v ) ); }\n"
		"static inline half pow_h( half x, " << precType << " y ) { return half( pow( float( x ), y ) ); }\n"
		"static inline half pow10_h( " << precType << " v ) { return half( pow( 10.0, v ) ); }\n"
		"\n"
		"static inline " << precMat33 << " mult_f33_f33( const " << precMat33 << " &a, const " << precMat33 << " &b ) { return a * b; }\n"
		"static inline " << precMat44 << " mult_f44_f44( const " << precMat44 << " &a, const " << precMat44 << " &b ) { return a * b; }\n"
		"static inline " << precMat33 << " mult_f_f33( " << precType << " a, const " << precMat33 << " &b ) { return a * b; }\n"
		"static inline " << precMat44 << " mult_f_f44( " << precType << " a, const " << precMat44 << " &b ) { return a * b; }\n"
		"static inline " << precMat33 << " add_f33_f33( const " << precMat33 << " &a, const " << precMat33 << " &b ) { return a + b; }\n"
		"static inline " << precMat44 << " add_f44_f44( const " << precMat44 << " &a, const " << precMat44 << " &b ) { return a + b; }\n"
		"static inline " << precMat33 << " invert_f33( const " << precMat33 << " &a ) { return a.inverse(); }\n"
		"static inline " << precMat44 << " invert_f44( const " << precMat44 << " &a ) { return a.inverse(); }\n"
		"static inline " << precMat33 << " transpose_f33( const " << precMat33 << " &a ) { return a.transposed(); }\n"
		"static inline " << precMat44 << " transpose_f44( const " << precMat44 << " &a ) { return a.transposed(); }\n"
		"static inline " << precVec3 << " mult_f3_f33( const " << precVec3 << " &a, const " << precMat33 << " &b ) { return a * b; }\n"
		"static inline " << precVec3 << " mult_f3_f44( const " << precVec3 << " &a, const " << precMat44 << " &b ) { return a * b; }\n"
		"static inline " << precVec3 << " mult_f_f3( " << precType << " a, const " << precVec3 << " &b ) { return a * b; }\n"
		"static inline " << precVec3 << " add_f3_f3( const " << precVec3 << " &a, const " << precVec3 << " &b ) { return a + b; }\n"
		"static inline " << precVec3 << " sub_f3_f3( const " << precVec3 << " &a, const " << precVec3 << " &b ) { return a - b; }\n"
		"static inline " << precVec3 << " cross_f3_f3( const " << precVec3 << " &a, const " << precVec3 << " &b ) { return a.cross( b ); }\n"
		"static inline " << precType << " dot_f3_f3( const " << precVec3 << " &a, const " << precVec3 << " &b ) { return a.dot( b ); }\n"
		"static inline " << precType << " length_f3( const " << precVec3 << " &a ) { return a.length(); }\n"
		"\n"
		"namespace {\n"
		"static inline " << precType << " __cspace_f( " << precType << " x ) { if ( x > " << precType << "(0.008856) ) return pow( x, " << precType << "(1.0 / 3.0) ); return " << precType << "(7.787) * x + " << precType << "(16.0 / 116.0); }\n"
		"static inline " << precType << " __cspace_fInverse( " << precType << " t ) { if ( t > " << precType << "(0.206892) ) return t * t * t; return " << precType << "(1.0/7.787) * ( t - " << precType << "(16.0/116.0) ); }\n"
		"static inline " << precType << " __cspace_uprime( const " << precVec3 << " &XYZ ) { return ( XYZ.x * " << precType << "(4) ) / ( XYZ.x + " << precType << "(15) * XYZ.y + " << precType << "(3) * XYZ.z ); }\n"
		"static inline " << precType << " __cspace_vprime( const " << precVec3 << " &XYZ ) { return ( XYZ.y * " << precType << "(9) ) / ( XYZ.x + " << precType << "(15) * XYZ.y + " << precType << "(3) * XYZ.z ); }\n"
		"} // empty namespace\n\n"
		"static inline " << precMat44 << " RGBtoXYZ( const Chromaticities &chroma, " << precType << " Y )\n"
		"{\n"
		"    static const " << precType << " one = " << precType << "(1);\n"
		"    " << precType << " X = chroma.white.x * Y / chroma.white.y;\n"
		"    " << precType << " Z = (one - chroma.white.x - chroma.white.y) * Y / chroma.white.y;\n"
		"    " << precType << " d = chroma.red.x * (chroma.blue.y - chroma.green.y) + chroma.blue.x * (chroma.green.y - chroma.red.y) + chroma.green.x * (chroma.red.y - chroma.blue.y);\n"
		"    " << precType << " Sr = (X * (chroma.blue.y - chroma.green.y) - chroma.green.x * (Y * (chroma.blue.y - one) + chroma.blue.y * (X + Z)) + chroma.blue.x * (Y * (chroma.green.y - one) + chroma.green.y * (X + Z))) / d;\n"
		"    " << precType << " Sg = (X * (chroma.red.y - chroma.blue.y) + chroma.red.x * (Y * (chroma.blue.y - one) + chroma.blue.y * (X + Z)) - chroma.blue.x * (Y * (chroma.red.y - one) + chroma.red.y * (X + Z))) / d;\n"
		"    " << precType << " Sb = (X * (chroma.green.y - chroma.red.y) - chroma.red.x * (Y * (chroma.green.y - one) + chroma.green.y * (X + Z)) + chroma.green.x * (Y * (chroma.red.y - one) + chroma.red.y * (X + Z))) / d;\n"
		"    " << precMat44 << " M;\n"
		"    M[0][0] = Sr * chroma.red.x;\n"
		"    M[0][1] = Sr * chroma.red.y;\n"
		"    M[0][2] = Sr * (1 - chroma.red.x - chroma.red.y);\n"
		"    M[1][0] = Sg * chroma.green.x;\n"
		"    M[1][1] = Sg * chroma.green.y;\n"
		"    M[1][2] = Sg * (1 - chroma.green.x - chroma.green.y);\n"
		"    M[2][0] = Sb * chroma.blue.x;\n"
		"    M[2][1] = Sb * chroma.blue.y;\n"
		"    M[2][2] = Sb * (1 - chroma.blue.x - chroma.blue.y);\n"
		"    return M;\n"
		"}\n"
		"static inline " << precMat44 << " XYZtoRGB( const Chromaticities &chroma, " << precType << " Y ) { return RGBtoXYZ( chroma, Y ).inverse(); }\n"
		"static inline " << precVec3 << " XYZtoLuv( const " << precVec3 << " &XYZ, const " << precVec3 << " &XYZn )\n"
		"{\n"
		"    " << precType << " Lstar = " << precType << "(116) * __cspace_f( XYZ.y / XYZn.y ) - " << precType << "(16);\n"
		"    " << precType << " ustar = " << precType << "(13) * Lstar * ( __cspace_uprime( XYZ ) - __cspace_uprime( XYZn ) );\n"
		"    " << precType << " vstar = " << precType << "(13) * Lstar * ( __cspace_vprime( XYZ ) - __cspace_vprime( XYZn ) );\n"
		"    return " << precVec3 << "( Lstar, ustar, vstar );\n"
		"}\n"
		"static inline " << precVec3 << " LuvtoXYZ( const " << precVec3 << " &Luv, const " << precVec3 << " &XYZn )\n"
		"{\n"
		"    " << precType << " Lstar = Luv.x;\n"
		"    " << precType << " ustar = Luv.y;\n"
		"    " << precType << " vstar = Luv.z;\n"
		"    " << precType << " unprime = __cspace_uprime( XYZn );\n"
		"    " << precType << " vnprime = __cspace_vprime( XYZn );\n"
		"    " << precType << " fY = (Lstar + " << precType << "(16)) / " << precType << "(116);\n"
		"    " << precType << " Y = XYZn.y * __cspace_fInverse( fY );\n"
		"    " << precType << " d = " << precType << "(4) * (" << precType << "(13) * Lstar * vnprime + vstar);\n"
		"    " << precType << " X = " << precType << "(9) * (" << precType << "(13) * Lstar * unprime + ustar) * Y / d;\n"
		"    " << precType << " Z = -( " << precType << "(3) * ustar + " << precType << "(13) * Lstar * ( " << precType << "(-12) + " << precType << "(3) * unprime + " << precType << "(20) * vnprime ) + " << precType << "(20) * vstar ) * Y / d;\n"
		"    return " << precVec3 << "( X, Y, Z );\n"
		"}\n"
		"static inline " << precVec3 << " XYZtoLab( const " << precVec3 << " &XYZ, const " << precVec3 << " &XYZn )\n"
		"{\n"
		"    " << precType << " tmpY = __cspace_f( XYZ.y / XYZn.y );\n"
		"    " << precType << " Lstar = " << precType << "(116) * tmpY - " << precType << "(16);\n"
		"    " << precType << " astar = " << precType << "(500) * ( __cspace_f( XYZ.x / XYZn.x ) -  tmpY );\n"
		"    " << precType << " bstar = " << precType << "(200) * ( tmpY - __cspace_f( XYZ.z / XYZn.z ) );\n"
		"    return " << precVec3 << "( Lstar, astar, bstar );\n"
		"}\n"
		"static inline " << precVec3 << " LabtoXYZ( const " << precVec3 << " &Lab, const " << precVec3 << " &XYZn )\n"
		"{\n"
		"    " << precType << " Lstar = Lab.x;\n"
		"    " << precType << " astar = Lab.y;\n"
		"    " << precType << " bstar = Lab.z;\n"
		"    " << precType << " fY = (Lstar + " << precType << "(16)) / " << precType << "(116);\n"
		"    " << precType << " fX = astar / " << precType << "(500) + fY;\n"
		"    " << precType << " fZ = fY - bstar / " << precType << "(200);\n"
		"    " << precType << " X = XYZn.x * __cspace_fInverse( fX );\n"
		"    " << precType << " Y = XYZn.y * __cspace_fInverse( fY );\n"
		"    " << precType << " Z = XYZn.z * __cspace_fInverse( fZ );\n"
		"    return " << precVec3 << "( X, Y, Z );\n"
		"}\n"
		"\n"
		"static inline " << precType << " lookup1D( " << precType << " table[], int size, " << precType << " pMin, " << precType << " pMax, " << precType << " p )\n"
		"{\n"
		"    int iMax = size - 1;\n"
		"    " << precType << " r = ( clamp( p, pMin, pMax ) - pMin ) / ( pMax - pMin ) * iMax;\n"
		"    int i = static_cast<int>( r );\n"
		"    " << precType << " u = r - static_cast<" << precType << ">( i );\n"
		"    " << precType << " t0 = table[i];\n"
		"    " << precType << " t1 = table[std::min( i + 1, iMax )];\n"
		"    return t0 + u * ( t1 - t0 );\n"
		"}\n"
		"} // namespace _ctlcc_\n\n";

	return libSetupB.str();
}


////////////////////////////////////////


void
CPPGenerator::pushBlock( void )
{
	newlineAndIndent();
	curStream() << '{';
	pushIndent();
}


////////////////////////////////////////


void
CPPGenerator::popBlock( void )
{
	popIndent();
	newlineAndIndent();
	curStream() << '}';
}


////////////////////////////////////////


void
CPPGenerator::module( CodeLContext &ctxt, const CodeModuleNode &m )
{
	Module *oldModule = myCurModule;
	myCurModule = ctxt.module();
	std::string mName = cleanName( myCurModule->name() );

	extractLiteralConstants( m.constants, ctxt );
	pushStream( myCodeStream );

	newlineAndIndent();
	curStream() << "// Module " << ctxt.module()->name() << " ("
				<< ctxt.fileName() << ")\n";
	newlineAndIndent();

	curStream() << "namespace " << mName << " {";
	newlineAndIndent();

	++myInModuleInit;
	myCurModuleInit.push_back( std::vector<std::string>() );

	const std::vector< std::pair< std::string, MemberVector > > &theStructs =
		ctxt.structs();
	if ( ! theStructs.empty() )
	{
		for ( size_t i = 0, N = theStructs.size(); i != N; ++i )
		{
			const std::pair< std::string, MemberVector > &s = theStructs[i];
			std::string n = removeNSQuals( s.first );
			newlineAndIndent();
			curStream() << "struct " << n;
			pushBlock();
			for ( size_t m = 0, M = s.second.size(); m != M; ++m )
			{
				newlineAndIndent();
				const Member &mem = s.second[m];
				variable( ctxt, mem.name, mem.type, false, false, false );
				curStream() << ';';
			}
			popBlock();
			curStream() << ";\n";
		}
	}

	// Forward declare any functions used in initializing variables
	FunctionNodePtr function = m.functions;
	++myDoForwardDecl;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}
	--myDoForwardDecl;
	newlineAndIndent();

	std::cout << "Generating consts..." << std::endl;
	StatementNodePtr consts = m.constants;
	while ( consts )
	{
		consts->generateCode( ctxt );
		consts = consts->next;
	}

	if ( m.constants )
		newlineAndIndent();
	--myInModuleInit;

	std::cout << "Generating module init..." << std::endl;
	if ( ! myCurModuleInit.back().empty() )
	{
		const std::vector<std::string> &initVals = myCurModuleInit.back();

		newlineAndIndent();
		curStream() << "struct __ctlcc_InitVals_" << mName;
		pushBlock();
		newlineAndIndent();
		curStream() << "__ctlcc_InitVals_" << mName << "( void )";
		pushBlock();
		for ( size_t i = 0, N = initVals.size(); i != N; ++i )
			curStream() << initVals[i] << '\n';
		newlineAndIndent();
		popBlock();
		popBlock();
		curStream() << ";";
		newlineAndIndent();
		curStream() << "static __ctlcc_InitVals_" << mName << " __ctlcc_GlobalInitializer_" << mName << ";\n\n";
	}

	function = m.functions;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}

	newlineAndIndent();
	curStream() << "} // namespace " << mName;
	newlineAndIndent();

	myCurModuleInit.pop_back();
	popStream();

	myCurModule = oldModule;
}


////////////////////////////////////////


void
CPPGenerator::function( CodeLContext &ctxt, const CodeFunctionNode &f )
{
	CodeFunctionTypePtr functionType = f.info->functionType();
	const ParamVector &params = functionType->parameters();

	if ( functionType->returnVarying() )
		std::cout << "Check on what it means to return varying..." << std::endl;

	if ( myDoForwardDecl > 0 )
	{
		// if we aren't used in initializing constants, we can be
		// declared inline and don't need a forward decl
		if ( myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
			return;
	}

	std::string funcName = removeNSQuals( f.name );
	bool isMain = funcName == ctxt.module()->name();
	if ( funcName == "main" )
	{
		funcName = myCurModule->name();
		isMain = true;
	}

	if ( isMain )
	{
		std::string nsName = myCurModule->name() + "::" + funcName;
		registerMainRoutine( funcName, nsName, f.info );

		// and put it in a header file in case someone cares
		// about that
		pushStream( myHeaderStream );
		newlineAndIndent();
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

		curStream() << ' ' << funcName << "( ";
		bool notfirst = false;
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			const Param &parm = params[i];
			if ( notfirst )
				curStream() << ", ";
			else
				notfirst = true;

			variable( ctxt, parm.name, parm.type,
					  parm.access == RWA_READ, true,
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

			if ( needsSizeArgument( parm.type ) )
				curStream() << ", int " << parm.name << "_size";
		}
		curStream() << " );";
		popStream();
	}
	else if ( myDoForwardDecl > 0 )
	{
		newlineAndIndent();
		curStream() << "static ";
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

		curStream() << ' ' << funcName << "( ";
		bool notfirst = false;
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			const Param &parm = params[i];
			if ( notfirst )
				curStream() << ", ";
			else
				notfirst = true;

			variable( ctxt, parm.name, parm.type,
					  parm.access == RWA_READ, true,
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

			if ( needsSizeArgument( parm.type ) )
				curStream() << ", int " << parm.name << "_size";
		}
		curStream() << " );";
		return;
	}

	newlineAndIndent();
	if ( ! isMain )
	{
		curStream() << "static ";
	
		if ( myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
			curStream() << "inline ";
	}

	variable( ctxt, std::string(), functionType->returnType(),
			  false, false, false );
	newlineAndIndent();

	curStream() << funcName << "( ";
	bool notfirst = false;
	for ( size_t i = 0, N = params.size(); i != N; ++i )
	{
		const Param &parm = params[i];
		if ( notfirst )
			curStream() << ", ";
		else
			notfirst = true;

		variable( ctxt, parm.name, parm.type,
				  parm.access == RWA_READ, true,
				  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

		if ( needsSizeArgument( parm.type ) )
			curStream() << ", int " << parm.name << "_size";
	}
	curStream() << " )";
	pushBlock();
	++myInFunction;
	StatementNodePtr bodyNode = f.body;
	while ( bodyNode )
	{
		bodyNode->generateCode( ctxt );
		bodyNode = bodyNode->next;
	}
	--myInFunction;
	popBlock();
	curStream() << "\n\n";
}


////////////////////////////////////////


void
CPPGenerator::variable( CodeLContext &ctxt, const CodeVariableNode &v )
{
	if ( myInModuleInit > 0 )
	{
		std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );

		// We'll just put the literal in directly
		if ( i != myGlobalLiterals.end() )
			return;

		bool doConst = ! v.info->isWritable();
		bool overrideInit = false;
		if ( ! myCPP11Mode )
		{
			// in C++11 we can use initializer lists or constructors
			// for everything. In old c++, some things have to be
			// initialized in a function. if we use one of those, we
			// can't initialize ourselves that quickly...
			if ( usesUninitLocalGlobals( v.initialValue ) )
			{
				doConst = false;
				overrideInit = true;
			}
		}

		std::stringstream varDeclB;
		pushStream( varDeclB );
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   doConst, false, false );
		popStream();
		std::string varDecl = varDeclB.str();
		bool doEmit = true;
		if ( overrideInit )
			initT = FUNC;

		if ( v.name.find( '$' ) != std::string::npos )
		{
			varDecl = v.name;
			
			std::string initVal;
			if ( v.initialValue )
			{
				if ( initT == FUNC )
				{
					if ( ! v.initialValue.cast<NameNode>() )
						throw std::logic_error( "NYI: complex default value handling for function type" );
				}
				std::stringstream initB;
				pushStream( initB );
				myCurInitType = initT;
				v.initialValue->generateCode( ctxt );
				myCurInitType = NONE;
				popStream();
				initVal = initB.str();
				if ( initT == CTOR )
				{
					std::stringstream typeB;
					pushStream( typeB );
					variable( ctxt, std::string(), v.info->type(),
							  false, false, false );
					popStream();
					initVal = typeB.str() + "( " + initVal + " )";
				}
			}
			myDefaultMappings[varDecl] = initVal;
			doEmit = false;
		}

		if ( doEmit )
		{
			myGlobalInitType[v.name] = initT;
			myGlobalVariables.insert( v.name );
			newlineAndIndent();
			curStream() << varDecl;
			doInit( initT, ctxt, v.initialValue, v.name );
		}
	}
	else
	{
		newlineAndIndent();
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   ! v.info->isWritable(), false, false );

		doInit( initT, ctxt, v.initialValue, v.name );
	}
}


////////////////////////////////////////


void
CPPGenerator::assignment( CodeLContext &ctxt, const CodeAssignmentNode &v )
{
	newlineAndIndent();
	v.lhs->generateCode( ctxt );
	curStream() << " = ";
	v.lhs->type->generateCastFrom( v.rhs, ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::expr( CodeLContext &ctxt, const CodeExprStatementNode &v )
{
	newlineAndIndent();
	v.expr->generateCode( ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void CPPGenerator::cond( CodeLContext &ctxt, const CodeIfNode &v )
{
	if ( myInElse > 0 )
		curStream() << ' ';
	else
		newlineAndIndent();
	curStream() << "if ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";

	StatementNodePtr tPath = v.truePath;
	StatementNodePtr fPath = v.falsePath;

	if ( tPath && ! tPath->next )
	{
		pushIndent();
		tPath->generateCode( ctxt );
		popIndent();
	}
	else
	{
		pushBlock();
		while ( tPath )
		{
			tPath->generateCode( ctxt );
			tPath = tPath->next;
		}
		popBlock();
	}

	if ( fPath )
	{
		newlineAndIndent();
		curStream() << "else";
		if ( fPath.cast<CodeIfNode>() && ! fPath->next )
		{
			++myInElse;
			fPath->generateCode( ctxt );
			--myInElse;
		}
		else if ( ! fPath->next )
		{
			pushIndent();
			fPath->generateCode( ctxt );
			popIndent();
		}
		else
		{
			int oldV = myInElse;
			myInElse = 0;
			pushBlock();
			while ( fPath )
			{
				fPath->generateCode( ctxt );
				fPath = fPath->next;
			}
			popBlock();
			myInElse = oldV;
		}
	}
}


////////////////////////////////////////


void
CPPGenerator::retval( CodeLContext &ctxt, const CodeReturnNode &v )
{
	newlineAndIndent();
	curStream() << "return";
	if ( v.returnedValue )
	{
		curStream() << ' ';
		v.info->type()->generateCastFrom( v.returnedValue, ctxt );
	}
	curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::loop( CodeLContext &ctxt, const CodeWhileNode &v )
{
	newlineAndIndent();
	curStream() << "while ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";
	pushBlock();
	StatementNodePtr body = v.loopBody;
	while ( body )
	{
		body->generateCode( ctxt );
		body = body->next;
	}
	popBlock();
}


////////////////////////////////////////


void
CPPGenerator::binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v )
{
	// operator precedence in CTL is same as C++, but we will have
	// lost any parens during the parse stage, so we should
	// introduce them all the time just in case
	curStream() << '(';
	v.operandType->generateCastFrom( v.leftOperand, ctxt );
	curStream() << ' ';
	v.operandType->generateCode( const_cast<CodeBinaryOpNode *>( &v ), ctxt );
	curStream() << ' ';
	v.operandType->generateCastFrom( v.rightOperand, ctxt );
	curStream() << ')';
}


////////////////////////////////////////


void
CPPGenerator::unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v )
{
	v.type->generateCode( const_cast<CodeUnaryOpNode *>( &v ), ctxt );
	v.type->generateCastFrom( v.operand, ctxt );
}


////////////////////////////////////////


void
CPPGenerator::index( CodeLContext &ctxt, const CodeArrayIndexNode &v )
{
	v.array->generateCode( ctxt );
	IntTypePtr intType = ctxt.newIntType();
	curStream() << '[';
	intType->generateCastFrom( v.index, ctxt );
	curStream() << ']';
}


////////////////////////////////////////


void
CPPGenerator::member( CodeLContext &ctxt, const CodeMemberNode &v )
{
	v.obj->generateCode( ctxt );
	curStream() << '.' << v.member;
}


////////////////////////////////////////


void
CPPGenerator::size( CodeLContext &ctxt, const CodeSizeNode &v )
{
	std::cout << "Need to check if it's a function parameter and retrieve <name>_size, or just extract the size value and inject it" << std::endl;
	throw std::logic_error( "Function not yet implemented" );
}


////////////////////////////////////////


void
CPPGenerator::name( CodeLContext &ctxt, const CodeNameNode &v )
{
	std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );
	if ( i != myGlobalLiterals.end() )
		curStream() << i->second;
	else if ( v.info->isFunction() )
	{
		const Module *m = v.info->module();
		if ( m == myCurModule )
			curStream() << removeNSQuals( v.name );
		else if ( m )
			curStream() << cleanName( m->name() ) << "::" << removeNSQuals( v.name );
		else
			curStream() << "_ctlcc_::" << removeNSQuals( v.name );
	}
	else
	{
		if ( myGlobalVariables.find( v.name ) != myGlobalVariables.end() )
		{
			const Module *m = v.info->module();
			if ( m == myCurModule )
				curStream() << removeNSQuals( v.name );
			else if ( m )
				curStream() << cleanName( m->name() ) << "::" << removeNSQuals( v.name );
			else
			{
				// these are currently all #defines, so no namespace
				curStream() << removeNSQuals( v.name );
			}
		}
		else
			curStream() << removeNSQuals( v.name );
	}
}


////////////////////////////////////////


void
CPPGenerator::boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v )
{
	if ( v.value )
		curStream() << "true";
	else
		curStream() << "false";
}


////////////////////////////////////////


void
CPPGenerator::intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CPPGenerator::uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CPPGenerator::halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v )
{
	curStream() << "half( " << std::setprecision( std::numeric_limits<half>::digits ) << static_cast<float>( v.value ) << " )";
}


////////////////////////////////////////


void
CPPGenerator::floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v )
{
	curStream() << std::setprecision( std::numeric_limits<float>::digits ) << static_cast<float>( v.value );
}


////////////////////////////////////////


void
CPPGenerator::stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v )
{
	curStream() << '"' << escapeLiteral( v.value ) << '"';
}


////////////////////////////////////////


static bool
isStdPrintFunction( const NameNodePtr &n )
{
	return ( n->name == "::print_bool" ||
			 n->name == "::print_int" ||
			 n->name == "::print_unsigned_int" ||
			 n->name == "::print_half" ||
			 n->name == "::print_float" ||
			 n->name == "::print_string" );
}

////////////////////////////////////////


void
CPPGenerator::call( CodeLContext &ctxt, const CodeCallNode &v )
{
	bool isPrint = isStdPrintFunction( v.function );

	if ( isPrint )
		newlineAndIndent();

	v.function->generateCode( ctxt );
	if ( v.arguments.empty() )
		curStream() << "()";
	else
	{
		SymbolInfoPtr info = v.function->info;
		FunctionTypePtr functionType = info->functionType();
		const ParamVector &parameters = functionType->parameters();
		curStream() << "( ";
		size_t i = 0;
		for ( size_t N = v.arguments.size(); i != N; ++i )
		{
			if ( i >= parameters.size() )
				throw std::logic_error( "Too many arguments in function call" );

			if ( i > 0 )
				curStream() << ", ";

			parameters[i].type->generateCastFrom( v.arguments[i], ctxt );
			if ( needsSizeArgument( parameters[i].type ) )
			{
				curStream() << ", ";
				extractSizeAndAdd( v.arguments[i], ctxt );
			}
		}
		for ( size_t N = parameters.size(); i < N; ++i )
		{
			const Param &parm = parameters[i];

			if ( i > 0 )
				curStream() << ", ";

			if ( ! parm.defaultValue )
				throw std::logic_error( "Missing argument in function call (no default value)" );

			NameNodePtr n = parm.defaultValue.cast<NameNode>();
			std::string defVal;
			std::string namesp;
			if ( n )
			{
				const Module *m = n->info->module();
				if ( m && m != myCurModule )
					namesp = cleanName( m->name() ) + "::";

				defVal = n->name;
			}
			else
			{
				std::stringstream nameLookupB;
				pushStream( nameLookupB );
				parm.defaultValue->generateCode( ctxt );
				popStream();
				defVal = nameLookupB.str();
			}
			std::map<std::string, std::string>::const_iterator found = myDefaultMappings.find( defVal );
			if ( found != myDefaultMappings.end() )
				curStream() << namesp << found->second;
			else
				curStream() << defVal;
		}
		curStream() << " )";
	}
	if ( isPrint )
		curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::value( CodeLContext &ctxt, const CodeValueNode &v )
{
	size_t idx = 0;
	valueRecurse( ctxt, v.elements, v.type, idx, "$$$$" );
}


////////////////////////////////////////


void
CPPGenerator::startToBool( void )
{
	curStream() << "static_cast<bool>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToInt( void )
{
	curStream() << "static_cast<int>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToUint( void )
{
	curStream() << "static_cast<unsigned int>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToHalf( void )
{
	curStream() << "half( ";
}


////////////////////////////////////////


void
CPPGenerator::startToFloat( void )
{
	curStream() << "static_cast<" + getPrecisionType() + ">( ";
}


////////////////////////////////////////


void
CPPGenerator::endCoersion( void )
{
	curStream() << " )";
}


////////////////////////////////////////


void
CPPGenerator::emitToken( Token t )
{
	switch ( t )
	{
		case TK_AND: curStream() << "&&"; break;
		case TK_OR: curStream() << "||"; break;

		case TK_BITAND: curStream() << "&"; break;
		case TK_BITNOT: curStream() << "~"; break;
		case TK_BITOR: curStream() << "|"; break;
		case TK_BITXOR: curStream() << "^"; break;

		case TK_LEFTSHIFT: curStream() << "<<"; break;
		case TK_RIGHTSHIFT: curStream() << "<<"; break;
			
		case TK_DIV: curStream() << "/"; break;
		case TK_MINUS: curStream() << "-"; break;
		case TK_MOD: curStream() << "%"; break;
		case TK_PLUS: curStream() << "+"; break;
		case TK_TIMES: curStream() << "*"; break;

		case TK_EQUAL: curStream() << "=="; break;
		case TK_GREATER: curStream() << ">"; break;
		case TK_GREATEREQUAL: curStream() << ">="; break;
		case TK_LESS: curStream() << "<"; break;
		case TK_LESSEQUAL: curStream() << "<="; break;
		case TK_NOT: curStream() << "!"; break;

		default:
			break;
	}
}


////////////////////////////////////////


void
CPPGenerator::valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
							const DataTypePtr &t, size_t &index,
							const std::string &varName,
							bool isSubItem )
{
	ArrayType *arrayType = dynamic_cast<ArrayType *>( t.pointer() );
	if ( arrayType )
	{
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << '{';
		}
		

		size_t N = arrayType->size();
		bool lineEveryItem = ( N > 4 );

		pushIndent();
		if ( myCurInitType == FUNC )
		{
			std::string typeName, postDecl;
//			if ( ! canBeBuiltinType( arrayType ) )
			bool doCtor = false;
			if ( ! findBuiltinType( typeName, postDecl, arrayType, ctxt ) )
			{
				newlineAndIndent();
				curStream() << varName << ".resize( " << arrayType->size() << " );";
			}
			else
				doCtor = true;

			if ( doCtor && isSubItem && postDecl.empty() )
			{
				myCurInitType = CTOR;
				// we're a sub-type, we can treat this as a CTOR scenario
				curStream() << varName << " = " << typeName << "( ";
				for (int i = 0; i < arrayType->size(); ++i)
				{
					if ( i > 0 )
						curStream() << ',';
					if ( lineEveryItem )
						newlineAndIndent();
					else if ( i > 0 )
						curStream() << ' ';

					valueRecurse( ctxt, elements, arrayType->elementType(), index, varName, true );
				}
				curStream() << " );";
				myCurInitType = FUNC;
			}
			else
			{
				std::stringstream nameB;
				nameB << varName << "[" << arrayType->size() << "]";
				std::string varNameSubscript = nameB.str();
				std::string::size_type startPos = varNameSubscript.find_last_of( '[' ) + 1;
				std::string::size_type endPos = varNameSubscript.find_last_of( ']' );
				for (int i = 0; i < arrayType->size(); ++i)
				{
					int curOut = i;
					bool done = false;
					for ( std::string::size_type x = endPos - 1; x >= startPos; --x )
					{
						if ( done )
							varNameSubscript[x] = ' ';
						else
						{
							varNameSubscript[x] = '0' + (curOut % 10);
							curOut /= 10;
							if ( curOut == 0 )
								done = true;
						}
					}

					newlineAndIndent();
					valueRecurse( ctxt, elements, arrayType->elementType(), index, varNameSubscript, true );
				}
			}
		}
		else
		{
			for (int i = 0; i < arrayType->size(); ++i)
			{
				if ( i > 0 )
					curStream() << ',';
				if ( lineEveryItem )
					newlineAndIndent();
				else if ( i > 0 )
					curStream() << ' ';

				valueRecurse( ctxt, elements, arrayType->elementType(), index, varName, true );
			}
		}
		popIndent();
		
		if ( myCurInitType == ASSIGN )
		{
			if ( lineEveryItem )
			{
				newlineAndIndent();
				curStream() << "}";
			}
			else
				curStream() << " }";
		}

		return;
	}

	StructType *structType = dynamic_cast<StructType *>( t.pointer() );
	if ( structType )
	{
		if ( myCurInitType == ASSIGN )
			curStream() << '{';

		if ( myCurInitType == FUNC )
		{
			for( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				std::string name = varName + "." + it->name;

				newlineAndIndent();

				valueRecurse( ctxt, elements, it->type, index, name, true );
			}
		}
		else
		{
			pushIndent();
			for( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				if ( it != structType->members().begin() )
					curStream() << ", ";

				valueRecurse( ctxt, elements, it->type, index, varName, true );
			}
			popIndent();
		}
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << "}";
		}
		return;
	}

	if ( myCurInitType == FUNC )
	{
		curStream() << varName << " = ";
		t->generateCastFrom( elements[index], ctxt );
		curStream() << ';';
	}
	else
		t->generateCastFrom( elements[index], ctxt );
	++index;
}


////////////////////////////////////////


CPPGenerator::InitType
CPPGenerator::variable( CodeLContext &ctxt,
						const std::string &name, const DataTypePtr &t,
						bool isConst, bool isInput, bool isWritable )
{
	InitType retval = ASSIGN;

	std::string postDecl;
	switch ( t->cDataType() )
	{
		case VoidTypeEnum:
			curStream() << "void";
			break;
		case BoolTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "bool";
			break;
		case IntTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "int";
			break;
		case UIntTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "unsigned int";
			break;
		case HalfTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "half";
			break;
		case FloatTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << getPrecisionType();
			break;
		case StringTypeEnum:
			if ( isConst )
				curStream() << "const std:string";
			else
				curStream() << "std::string";
			retval = CTOR;
			break;
		case StructTypeEnum:
		{
			StructTypePtr structType = t.cast<StructType>();
			if ( myCPP11Mode &&  isConst )
				curStream() << "const ";
			curStream() << removeNSQuals( structType->name() );
			isWritable = isInput || isWritable;
			if ( myCPP11Mode )
				retval = ASSIGN;
			else
				retval = FUNC;
			break;
		}
		case ArrayTypeEnum:
		{
			ArrayTypePtr arrayType = t.cast<ArrayType>();

			std::string typeName;

			if ( myCPP11Mode )
				retval = ASSIGN;
			else
				retval = FUNC;

			if ( ! findBuiltinType( typeName, postDecl, arrayType, ctxt ) )
			{
				std::stringstream x;

				x << "std::vector< ";
				pushStream( x );
				variable( ctxt, std::string(), arrayType->elementType(),
						  false, false, false );
				popStream();
				x << " >";
				typeName = x.str();

				if ( retval == ASSIGN && isConst )
					curStream() << "const ";
			}
			else
			{
				if ( ! postDecl.empty() )
					retval = ASSIGN;
				else
					retval = CTOR;

				if ( isConst )
					curStream() << "const ";
			}
			curStream() << typeName;
			isWritable = ( isInput || isWritable ) && postDecl.empty();
			break;
		}
	}

	if ( isWritable )
		curStream() << " &";

	if ( ! name.empty() )
	{
		if ( ! isWritable )
			curStream() << ' ';
		curStream() << removeNSQuals( name );
	}
	curStream() << postDecl;

	return retval;
}


////////////////////////////////////////


void
CPPGenerator::doInit( InitType initT, CodeLContext &ctxt,
					  const ExprNodePtr &initV, const std::string &varName )
{
	myCurInitType = initT;
	if ( initV )
	{
		switch ( initT )
		{
			case CTOR:
				curStream() << "( ";
				initV->generateCode( ctxt );
				curStream() << " );";
				break;
			case FUNC:
			{
				curStream() << ';';
				std::stringstream varAssignB;
				pushStream( varAssignB );
				if ( ! dynamic_cast<ValueNode *>( initV.pointer() ) )
				{
					pushIndent();
					addIndent();
					curStream() << "$$$$ = ";
					initV->generateCode( ctxt );
					curStream() << ';';
					popIndent();
				}
				else
					initV->generateCode( ctxt );
				popStream();

				std::string initVal = varAssignB.str();
				replaceInit( initVal, varName );

				if ( myInFunction )
				{
					pushBlock();
					curStream() << initVal;
					popBlock();
				}
				else if ( varName.find_first_of( '$' ) == std::string::npos )
					myCurModuleInit.back().push_back( initVal );
				break;
			}
			case ASSIGN:
				curStream() << " = ";
				initV->generateCode( ctxt );
				curStream() << ";";
				break;
			case NONE:
				throw std::logic_error( "Invalid initialization type" );
				break;
		}
	}
	else
		curStream() << ';';
	myCurInitType = NONE;
}


////////////////////////////////////////


void
CPPGenerator::replaceInit( std::string &initS, const std::string &name )
{
	std::string::size_type replB = initS.find( "$$$$" );
	while ( replB != std::string::npos )
	{
		initS.replace( replB, 4, name );
		replB = initS.find( "$$$$", replB + name.size() );
	}
}


////////////////////////////////////////


bool
CPPGenerator::findBuiltinType( std::string &typeName,
							   std::string &postDecl,
							   const ArrayTypePtr &arrayType,
							   CodeLContext &ctxt )
{
	int sx = arrayType->size();

	if ( dynamic_cast<FloatType *>( arrayType->elementType().pointer() ) )
	{
		ArrayType *x = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
		// not x makes sure we are of size 1 (performance critical code, avoid
		// creation of RcPtr type w/ lock)
		if ( ! x && sx > 0 )
		{
			switch ( sx )
			{
				case 2:
				case 3:
				case 4:
					typeName = getVectorType( sx );
					break;
				default:
				{
					// just do a C array of the low level type
					typeName = getPrecisionType();
					std::stringstream pB;
					pB << '[' << sx << ']';
					postDecl = pB.str();
					break;
				}
			}
		}
		return ! typeName.empty();
	}

	ArrayType *subArray = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
	if ( subArray && dynamic_cast<FloatType *>( subArray->elementType().pointer() ) &&
		 sx == subArray->size() )
	{
		if ( sx == 3 )
			typeName = getMatrixType( 3 );
		else if ( sx == 4 )
			typeName = getMatrixType( 4 );

		return ! typeName.empty();
	}

	if ( dynamic_cast<IntType *>( arrayType->elementType().pointer() ) )
	{
		ArrayType *x = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
		int sx = arrayType->size();
		// not x makes sure we are of size 1 (performance critical code, avoid
		// creation of RcPtr type w/ lock)
		if ( ! x && sx > 0 )
		{
			switch ( sx )
			{
				case 2:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V2i");
					break;
				case 3:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V3i");
					break;
				case 4:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V4i");
					break;
				default:
				{
					// just do a C array of the low level type
					typeName = "int";
					std::stringstream pB;
					pB << '[' << sx << ']';
					postDecl = pB.str();
					break;
				}
			}
		}
		return ! typeName.empty();
	}

	return ! typeName.empty();
}


////////////////////////////////////////


bool
CPPGenerator::canBeBuiltinType( const ArrayType *arrayType )
{
	if ( dynamic_cast<FloatType *>( arrayType->elementType().pointer() ) ||
		 dynamic_cast<IntType *>( arrayType->elementType().pointer() ) )
	{
		ArrayType *x = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
		// not x makes sure we are of size 1 (performance critical code, avoid
		// creation of RcPtr type w/ lock)
		if ( ! x && arrayType->size() > 0 )
			return true;
		return false;
	}
	
	ArrayType *subArray = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
	if ( subArray && dynamic_cast<FloatType *>( subArray->elementType().pointer() ) &&
		 arrayType->size() == subArray->size() )
	{
		if ( arrayType->size() == 3 || arrayType->size() == 4 )
			return true;
	}

	return false;
}


////////////////////////////////////////


bool
CPPGenerator::checkNeedInitInModuleInit( const ExprNodePtr &initV, bool deep )
{
	if ( ! initV )
		return false;

	if ( isAllLiterals( initV ) )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( initV.pointer() );
	if ( val )
	{
		bool retval = false;
		// make sure we check every value for all possible functions
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			bool a = checkNeedInitInModuleInit( val->elements[i] );
			if ( a && ! retval )
			{
				if ( ! deep )
					return true;
				retval = true;
			}
		}
		return true;
	}

	CallNode *c = dynamic_cast<CallNode *>( initV.pointer() );
	if ( c )
	{
		bool retval = false;
		SymbolInfoPtr info = c->function->info;
		if ( info->module() == myCurModule )
		{
			myFuncsUsedInInit.insert( c->function->name );
			if ( deep )
				retval = true;
			else
				return true;
		}

		if ( ! c->arguments.empty() )
		{
			// make sure we check every value for all possible functions
			for ( size_t i = 0, N = c->arguments.size(); i != N; ++i )
			{
				bool needed = checkNeedInitInModuleInit( c->arguments[i] );
				if ( needed && ! retval )
				{
					if ( ! deep )
						return true;
					retval = true;
				}
			}
		}
		return retval;
	}

	BinaryOpNodePtr bOp = initV.cast<BinaryOpNode>();
	if ( bOp )
	{
		// make sure both run to pick up
		// any functions used...
		bool l = checkNeedInitInModuleInit( bOp->leftOperand );
		bool r = checkNeedInitInModuleInit( bOp->rightOperand );
		return l || r;
	}
	UnaryOpNodePtr uOp = initV.cast<UnaryOpNode>();
	if ( uOp )
	{
		return checkNeedInitInModuleInit( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


bool
CPPGenerator::isAllLiterals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( dynamic_cast<LiteralNode *>( v.pointer() ) )
		return true;

	if ( dynamic_cast<NameNode *>( v.pointer() ) )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( v.pointer() );
	if ( val )
	{
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			if ( ! isAllLiterals( val->elements[i] ) )
				return false;
		}
		return true;
	}

	BinaryOpNodePtr bOp = v.cast<BinaryOpNode>();
	if ( bOp )
	{
		if ( ! isAllLiterals( bOp->leftOperand ) ||
			 ! isAllLiterals( bOp->rightOperand ) )
			return false;

		return true;
	}
	UnaryOpNodePtr uOp = v.cast<UnaryOpNode>();
	if ( uOp )
	{
		return isAllLiterals( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


bool
CPPGenerator::usesUninitLocalGlobals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( dynamic_cast<LiteralNode *>( v.pointer() ) )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( v.pointer() );
	if ( val )
	{
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			if ( usesUninitLocalGlobals( val->elements[i] ) )
				return true;
		}
		return false;
	}

	NameNode *namePtr = dynamic_cast<NameNode *>( v.pointer() );
	if ( namePtr )
	{
		if ( namePtr->info->module() == myCurModule )
		{
			std::map<std::string, InitType>::const_iterator i = myGlobalInitType.find( namePtr->name );
			if ( i != myGlobalInitType.end() )
			{
				if ( i->second == FUNC )
					return true;
			}
		}

		return false;
	}

	CallNodePtr callPtr = v.cast<CallNode>();
	if ( callPtr )
	{
		for ( size_t i = 0, N = callPtr->arguments.size(); i != N; ++i )
		{
			if ( usesUninitLocalGlobals( callPtr->arguments[i] ) )
				return true;
		}

		return false;
	}

	BinaryOpNodePtr bOp = v.cast<BinaryOpNode>();
	if ( bOp )
	{
		if ( usesUninitLocalGlobals( bOp->leftOperand ) ||
			 usesUninitLocalGlobals( bOp->rightOperand ) )
			return true;

		return false;
	}
	UnaryOpNodePtr uOp = v.cast<UnaryOpNode>();
	if ( uOp )
	{
		return usesUninitLocalGlobals( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


void
CPPGenerator::extractLiteralConstants( const StatementNodePtr &consts,
									   CodeLContext &ctxt )
{
	StatementNodePtr curConst = consts;
	while ( curConst )
	{
		VariableNodePtr var = curConst.cast<VariableNode>();
		if ( isAllLiterals( var->initialValue ) )
		{
			if ( ! var->initialValue.cast<ValueNode>() )
			{
				std::stringstream x;
				pushStream( x );
				var->initialValue->generateCode( ctxt );
				popStream();
				myGlobalLiterals[var->name] = x.str();
			}
		}
		else if ( var->initialValue )
		{
			// don't care about the result, but need to get the function names
			// into the func used table
			checkNeedInitInModuleInit( var->initialValue, true );
		}
		curConst = curConst->next;
	}
}


////////////////////////////////////////


bool
CPPGenerator::needsSizeArgument( const DataTypePtr &p )
{
	return false;
}


////////////////////////////////////////


void
CPPGenerator::extractSizeAndAdd( const ExprNodePtr &p, CodeLContext &ctxt )
{
}

} // namespace Ctl

