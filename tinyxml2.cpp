

#include "tinyxml2.h"

#include <new>		// yes, this one new style header, is in the Android SDK.
#if defined(ANDROID_NDK) || defined(__QNXNTO__)
#   include <stddef.h>
#   include <stdarg.h>
#else
#   include <cstddef>
#   include <cstdarg>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) && (!defined WINCE)

	static inline int TIXML_SNPRINTF( char* buffer, size_t size, const char* format, ... )
	{
		va_list va;
		va_start( va, format );
		int result = vsnprintf_s( buffer, size, _TRUNCATE, format, va );
		va_end( va );
		return result;
	}

	static inline int TIXML_VSNPRINTF( char* buffer, size_t size, const char* format, va_list va )
	{
		int result = vsnprintf_s( buffer, size, _TRUNCATE, format, va );
		return result;
	}

	#define TIXML_VSCPRINTF	_vscprintf
	#define TIXML_SSCANF	sscanf_s
#elif defined _MSC_VER
	// Microsoft Visual Studio 2003 and earlier or WinCE
	#define TIXML_SNPRINTF	_snprintf
	#define TIXML_VSNPRINTF _vsnprintf
	#define TIXML_SSCANF	sscanf
	#if (_MSC_VER < 1400 ) && (!defined WINCE)
		// Microsoft Visual Studio 2003 and not WinCE.
		#define TIXML_VSCPRINTF   _vscprintf // VS2003's C runtime has this, but VC6 C runtime or WinCE SDK doesn't have.
	#else
		// Microsoft Visual Studio 2003 and earlier or WinCE.
		static inline int TIXML_VSCPRINTF( const char* format, va_list va )
		{
			int len = 512;
			for (;;) {
				len = len*2;
				char* str = new char[len]();
				const int required = _vsnprintf(str, len, format, va);
				delete[] str;
				if ( required != -1 ) {
					TIXMLASSERT( required >= 0 );
					len = required;
					break;
				}
			}
			TIXMLASSERT( len >= 0 );
			return len;
		}
	#endif
#else
	// GCC version 3 and higher
	//#warning( "Using sn* functions." )
	#define TIXML_SNPRINTF	snprintf
	#define TIXML_VSNPRINTF	vsnprintf
	static inline int TIXML_VSCPRINTF( const char* format, va_list va )
	{
		int len = vsnprintf( 0, 0, format, va );
		TIXMLASSERT( len >= 0 );
		return len;
	}
	#define TIXML_SSCANF   sscanf
#endif


static const char LINE_FEED				= (char)0x0a;			// all line endings are normalized to LF
static const char LF = LINE_FEED;
static const char CARRIAGE_RETURN		= (char)0x0d;			// CR gets filtered out
static const char CR = CARRIAGE_RETURN;
static const char SINGLE_QUOTE			= '\'';
static const char DOUBLE_QUOTE			= '\"';

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
//	ef bb bf (Microsoft "lead bytes") - designates UTF-8

static const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
static const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
static const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

namespace tinyxml2
{

struct Entity {
    const char* pattern;
    int length;
    char value;
};

static const int NUM_ENTITIES = 5;
static const Entity entities[NUM_ENTITIES] = {
    { "quot", 4,	DOUBLE_QUOTE },
    { "amp", 3,		'&'  },
    { "apos", 4,	SINGLE_QUOTE },
    { "lt",	2, 		'<'	 },
    { "gt",	2,		'>'	 }
};


StrPair::~StrPair()
{
    Reset();
}


void StrPair::TransferTo( StrPair* other )
{
    if ( this == other ) {
        return;
    }
    // This in effect implements the assignment operator by "moving"
    // ownership (as in auto_ptr).

    TIXMLASSERT( other->_flags == 0 );
    TIXMLASSERT( other->_start == 0 );
    TIXMLASSERT( other->_end == 0 );

    other->Reset();

    other->_flags = _flags;
    other->_start = _start;
    other->_end = _end;

    _flags = 0;
    _start = 0;
    _end = 0;
}

void StrPair::Reset()
{
    if ( _flags & NEEDS_DELETE ) {
        delete [] _start;
    }
    _flags = 0;
    _start = 0;
    _end = 0;
}


void StrPair::SetStr( const char* str, int flags )
{
    Reset();
    size_t len = strlen( str );
    TIXMLASSERT( _start == 0 );
    _start = new char[ len+1 ];
    memcpy( _start, str, len+1 );
    _end = _start + len;
    _flags = flags | NEEDS_DELETE;
}


char* StrPair::ParseText( char* p, const char* endTag, int strFlags )
{
    TIXMLASSERT( endTag && *endTag );

    char* start = p;
    char  endChar = *endTag;
    size_t length = strlen( endTag );

    // Inner loop of text parsing.
    while ( *p ) {
        if ( *p == endChar && strncmp( p, endTag, length ) == 0 ) {
            Set( start, p, strFlags );
            return p + length;
        }
        ++p;
    }
    return 0;
}


char* StrPair::ParseName( char* p )
{
    if ( !p || !(*p) ) {
        return 0;
    }
    if ( !XMLUtil::IsNameStartChar( *p ) ) {
        return 0;
    }

    char* const start = p;
    ++p;
    while ( *p && XMLUtil::IsNameChar( *p ) ) {
        ++p;
    }

    Set( start, p, 0 );
    return p;
}


void StrPair::CollapseWhitespace()
{
    // Adjusting _start would cause undefined behavior on delete[]
    TIXMLASSERT( ( _flags & NEEDS_DELETE ) == 0 );
    // Trim leading space.
    _start = XMLUtil::SkipWhiteSpace( _start );

    if ( *_start ) {
        char* p = _start;	// the read pointer
        char* q = _start;	// the write pointer

        while( *p ) {
            if ( XMLUtil::IsWhiteSpace( *p )) {
                p = XMLUtil::SkipWhiteSpace( p );
                if ( *p == 0 ) {
                    break;    // don't write to q; this trims the trailing space.
                }
                *q = ' ';
                ++q;
            }
            *q = *p;
            ++q;
            ++p;
        }
        *q = 0;
    }
}


const char* StrPair::GetStr()
{
    TIXMLASSERT( _start );
    TIXMLASSERT( _end );
    if ( _flags & NEEDS_FLUSH ) {
        *_end = 0;
        _flags ^= NEEDS_FLUSH;

        if ( _flags ) {
            char* p = _start;	// the read pointer
            char* q = _start;	// the write pointer

            while( p < _end ) {
                if ( (_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == CR ) {
                    // CR-LF pair becomes LF
                    // CR alone becomes LF
                    // LF-CR becomes LF
                    if ( *(p+1) == LF ) {
                        p += 2;
                    }
                    else {
                        ++p;
                    }
                    *q++ = LF;
                }
                else if ( (_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == LF ) {
                    if ( *(p+1) == CR ) {
                        p += 2;
                    }
                    else {
                        ++p;
                    }
                    *q++ = LF;
                }
                else if ( (_flags & NEEDS_ENTITY_PROCESSING) && *p == '&' ) {
                    // Entities handled by tinyXML2:
                    // - special entities in the entity table [in/out]
                    // - numeric character reference [in]
                    //   &#20013; or &#x4e2d;

                    if ( *(p+1) == '#' ) {
                        const int buflen = 10;
                        char buf[buflen] = { 0 };
                        int len = 0;
                        char* adjusted = const_cast<char*>( XMLUtil::GetCharacterRef( p, buf, &len ) );
                        if ( adjusted == 0 ) {
                            *q = *p;
                            ++p;
                            ++q;
                        }
                        else {
                            TIXMLASSERT( 0 <= len && len <= buflen );
                            TIXMLASSERT( q + len <= adjusted );
                            p = adjusted;
                            memcpy( q, buf, len );
                            q += len;
                        }
                    }
                    else {
                        bool entityFound = false;
                        for( int i = 0; i < NUM_ENTITIES; ++i ) {
                            const Entity& entity = entities[i];
                            if ( strncmp( p + 1, entity.pattern, entity.length ) == 0
                                    && *( p + entity.length + 1 ) == ';' ) {
                                // Found an entity - convert.
                                *q = entity.value;
                                ++q;
                                p += entity.length + 2;
                                entityFound = true;
                                break;
                            }
                        }
                        if ( !entityFound ) {
                            // fixme: treat as error?
                            ++p;
                            ++q;
                        }
                    }
                }
                else {
                    *q = *p;
                    ++p;
                    ++q;
                }
            }
            *q = 0;
        }
        // The loop below has plenty going on, and this
        // is a less useful mode. Break it out.
        if ( _flags & NEEDS_WHITESPACE_COLLAPSING ) {
            CollapseWhitespace();
        }
        _flags = (_flags & NEEDS_DELETE);
    }
    TIXMLASSERT( _start );
    return _start;
}




// --------- XMLUtil ----------- //

const char* XMLUtil::ReadBOM( const char* p, bool* bom )
{
    TIXMLASSERT( p );
    TIXMLASSERT( bom );
    *bom = false;
    const unsigned char* pu = reinterpret_cast<const unsigned char*>(p);
    // Check for BOM:
    if (    *(pu+0) == TIXML_UTF_LEAD_0
            && *(pu+1) == TIXML_UTF_LEAD_1
            && *(pu+2) == TIXML_UTF_LEAD_2 ) {
        *bom = true;
        p += 3;
    }
    TIXMLASSERT( p );
    return p;
}


void XMLUtil::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
{
    const unsigned long BYTE_MASK = 0xBF;
    const unsigned long BYTE_MARK = 0x80;
    const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    if (input < 0x80) {
        *length = 1;
    }
    else if ( input < 0x800 ) {
        *length = 2;
    }
    else if ( input < 0x10000 ) {
        *length = 3;
    }
    else if ( input < 0x200000 ) {
        *length = 4;
    }
    else {
        *length = 0;    // This code won't convert this correctly anyway.
        return;
    }

    output += *length;

    // Scary scary fall throughs.
    switch (*length) {
        case 4:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 3:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 2:
            --output;
            *output = (char)((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
        case 1:
            --output;
            *output = (char)(input | FIRST_BYTE_MARK[*length]);
            break;
        default:
            TIXMLASSERT( false );
    }
}


const char* XMLUtil::GetCharacterRef( const char* p, char* value, int* length )
{
    // Presume an entity, and pull it out.
    *length = 0;

    if ( *(p+1) == '#' && *(p+2) ) {
        unsigned long ucs = 0;
        TIXMLASSERT( sizeof( ucs ) >= 4 );
        ptrdiff_t delta = 0;
        unsigned mult = 1;
        static const char SEMICOLON = ';';

        if ( *(p+2) == 'x' ) {
            // Hexadecimal.
            const char* q = p+3;
            if ( !(*q) ) {
                return 0;
            }

            q = strchr( q, SEMICOLON );

            if ( !q ) {
                return 0;
            }
            TIXMLASSERT( *q == SEMICOLON );

            delta = q-p;
            --q;

            while ( *q != 'x' ) {
                unsigned int digit = 0;

                if ( *q >= '0' && *q <= '9' ) {
                    digit = *q - '0';
                }
                else if ( *q >= 'a' && *q <= 'f' ) {
                    digit = *q - 'a' + 10;
                }
                else if ( *q >= 'A' && *q <= 'F' ) {
                    digit = *q - 'A' + 10;
                }
                else {
                    return 0;
                }
                TIXMLASSERT( digit >= 0 && digit < 16);
                TIXMLASSERT( digit == 0 || mult <= UINT_MAX / digit );
                const unsigned int digitScaled = mult * digit;
                TIXMLASSERT( ucs <= ULONG_MAX - digitScaled );
                ucs += digitScaled;
                TIXMLASSERT( mult <= UINT_MAX / 16 );
                mult *= 16;
                --q;
            }
        }
        else {
            // Decimal.
            const char* q = p+2;
            if ( !(*q) ) {
                return 0;
            }

            q = strchr( q, SEMICOLON );

            if ( !q ) {
                return 0;
            }
            TIXMLASSERT( *q == SEMICOLON );

            delta = q-p;
            --q;

            while ( *q != '#' ) {
                if ( *q >= '0' && *q <= '9' ) {
                    const unsigned int digit = *q - '0';
                    TIXMLASSERT( digit >= 0 && digit < 10);
                    TIXMLASSERT( digit == 0 || mult <= UINT_MAX / digit );
                    const unsigned int digitScaled = mult * digit;
                    TIXMLASSERT( ucs <= ULONG_MAX - digitScaled );
                    ucs += digitScaled;
                }
                else {
                    return 0;
                }
                TIXMLASSERT( mult <= UINT_MAX / 10 );
                mult *= 10;
                --q;
            }
        }
        // convert the UCS to UTF-8
        ConvertUTF32ToUTF8( ucs, value, length );
        return p + delta + 1;
    }
    return p+1;
}


void XMLUtil::ToStr( int v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%d", v );
}


void XMLUtil::ToStr( unsigned v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%u", v );
}


void XMLUtil::ToStr( bool v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%d", v ? 1 : 0 );
}

/*
	ToStr() of a number is a very tricky topic.
	https://github.com/leethomason/tinyxml2/issues/106
*/
void XMLUtil::ToStr( float v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%.8g", v );
}


void XMLUtil::ToStr( double v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%.17g", v );
}


bool XMLUtil::ToInt( const char* str, int* value )
{
    if ( TIXML_SSCANF( str, "%d", value ) == 1 ) {
        return true;
    }
    return false;
}

bool XMLUtil::ToUnsigned( const char* str, unsigned *value )
{
    if ( TIXML_SSCANF( str, "%u", value ) == 1 ) {
        return true;
    }
    return false;
}

bool XMLUtil::ToBool( const char* str, bool* value )
{
    int ival = 0;
    if ( ToInt( str, &ival )) {
        *value = (ival==0) ? false : true;
        return true;
    }
    if ( StringEqual( str, "true" ) ) {
        *value = true;
        return true;
    }
    else if ( StringEqual( str, "false" ) ) {
        *value = false;
        return true;
    }
    return false;
}


bool XMLUtil::ToFloat( const char* str, float* value )
{
    if ( TIXML_SSCANF( str, "%f", value ) == 1 ) {
        return true;
    }
    return false;
}

bool XMLUtil::ToDouble( const char* str, double* value )
{
    if ( TIXML_SSCANF( str, "%lf", value ) == 1 ) {
        return true;
    }
    return false;
}


char* XMLDocument::Identify( char* p, XMLNode** node )
{
    TIXMLASSERT( node );
    TIXMLASSERT( p );
    char* const start = p;
    p = XMLUtil::SkipWhiteSpace( p );
    if( !*p ) {
        *node = 0;
        TIXMLASSERT( p );
        return p;
    }

    // These strings define the matching patterns:
    static const char* xmlHeader		= { "<?" };
    static const char* commentHeader	= { "<!--" };
    static const char* cdataHeader		= { "<![CDATA[" };
    static const char* dtdHeader		= { "<!" };
    static const char* elementHeader	= { "<" };	// and a header for everything else; check last.

    static const int xmlHeaderLen		= 2;
    static const int commentHeaderLen	= 4;
    static const int cdataHeaderLen		= 9;
    static const int dtdHeaderLen		= 2;
    static const int elementHeaderLen	= 1;

    TIXMLASSERT( sizeof( XMLComment ) == sizeof( XMLUnknown ) );		// use same memory pool
    TIXMLASSERT( sizeof( XMLComment ) == sizeof( XMLDeclaration ) );	// use same memory pool
    XMLNode* returnNode = 0;
    if ( XMLUtil::StringEqual( p, xmlHeader, xmlHeaderLen ) ) {
        TIXMLASSERT( sizeof( XMLDeclaration ) == _commentPool.ItemSize() );
        returnNode = new (_commentPool.Alloc()) XMLDeclaration( this );
        returnNode->_memPool = &_commentPool;
        p += xmlHeaderLen;
    }
    else if ( XMLUtil::StringEqual( p, commentHeader, commentHeaderLen ) ) {
        TIXMLASSERT( sizeof( XMLComment ) == _commentPool.ItemSize() );
        returnNode = new (_commentPool.Alloc()) XMLComment( this );
        returnNode->_memPool = &_commentPool;
        p += commentHeaderLen;
    }
    else if ( XMLUtil::StringEqual( p, cdataHeader, cdataHeaderLen ) ) {
        TIXMLASSERT( sizeof( XMLText ) == _textPool.ItemSize() );
        XMLText* text = new (_textPool.Alloc()) XMLText( this );
        returnNode = text;
        returnNode->_memPool = &_textPool;
        p += cdataHeaderLen;
        text->SetCData( true );
    }
    else if ( XMLUtil::StringEqual( p, dtdHeader, dtdHeaderLen ) ) {
        TIXMLASSERT( sizeof( XMLUnknown ) == _commentPool.ItemSize() );
        returnNode = new (_commentPool.Alloc()) XMLUnknown( this );
        returnNode->_memPool = &_commentPool;
        p += dtdHeaderLen;
    }
    else if ( XMLUtil::StringEqual( p, elementHeader, elementHeaderLen ) ) {
        TIXMLASSERT( sizeof( XMLElement ) == _elementPool.ItemSize() );
        returnNode = new (_elementPool.Alloc()) XMLElement( this );
        returnNode->_memPool = &_elementPool;
        p += elementHeaderLen;
    }
    else {
        TIXMLASSERT( sizeof( XMLText ) == _textPool.ItemSize() );
        returnNode = new (_textPool.Alloc()) XMLText( this );
        returnNode->_memPool = &_textPool;
        p = start;	// Back it up, all the text counts.
    }

    TIXMLASSERT( returnNode );
    TIXMLASSERT( p );
    *node = returnNode;
    return p;
}


bool XMLDocument::Accept( XMLVisitor* visitor ) const
{
    TIXMLASSERT( visitor );
    if ( visitor->VisitEnter( *this ) ) {
        for ( const XMLNode* node=FirstChild(); node; node=node->NextSibling() ) {
            if ( !node->Accept( visitor ) ) {
                break;
            }
        }
    }
    return visitor->VisitExit( *this );
}


// --------- XMLNode ----------- //

XMLNode::XMLNode( XMLDocument* doc ) :
    _document( doc ),
    _parent( 0 ),
    _firstChild( 0 ), _lastChild( 0 ),
    _prev( 0 ), _next( 0 ),
    _memPool( 0 )
{
}


XMLNode::~XMLNode()
{
    DeleteChildren();
    if ( _parent ) {
        _parent->Unlink( this );
    }
}

const char* XMLNode::Value() const 
{
    // Catch an edge case: XMLDocuments don't have a a Value. Carefully return nullptr.
    if ( this->ToDocument() )
        return 0;
    return _value.GetStr();
}

void XMLNode::SetValue( const char* str, bool staticMem )
{
    if ( staticMem ) {
        _value.SetInternedStr( str );
    }
    else {
        _value.SetStr( str );
    }
}


void XMLNode::DeleteChildren()
{
    while( _firstChild ) {
        TIXMLASSERT( _lastChild );
        TIXMLASSERT( _firstChild->_document == _document );
        XMLNode* node = _firstChild;
        Unlink( node );

        DeleteNode( node );
    }
    _firstChild = _lastChild = 0;
}


void XMLNode::Unlink( XMLNode* child )
{
    TIXMLASSERT( child );
    TIXMLASSERT( child->_document == _document );
    TIXMLASSERT( child->_parent == this );
    if ( child == _firstChild ) {
        _firstChild = _firstChild->_next;
    }
    if ( child == _lastChild ) {
        _lastChild = _lastChild->_prev;
    }

    if ( child->_prev ) {
        child->_prev->_next = child->_next;
    }
    if ( child->_next ) {
        child->_next->_prev = child->_prev;
    }
	child->_parent = 0;
}


void XMLNode::DeleteChild( XMLNode* node )
{
    TIXMLASSERT( node );
    TIXMLASSERT( node->_document == _document );
    TIXMLASSERT( node->_parent == this );
    DeleteNode( node );
}


XMLNode* XMLNode::InsertEndChild( XMLNode* addThis )
{
    TIXMLASSERT( addThis );
    if ( addThis->_document != _document ) {
        TIXMLASSERT( false );
        return 0;
    }
    InsertChildPreamble( addThis );

    if ( _lastChild ) {
        TIXMLASSERT( _firstChild );
        TIXMLASSERT( _lastChild->_next == 0 );
        _lastChild->_next = addThis;
        addThis->_prev = _lastChild;
        _lastChild = addThis;

        addThis->_next = 0;
    }
    else {
        TIXMLASSERT( _firstChild == 0 );
        _firstChild = _lastChild = addThis;

        addThis->_prev = 0;
        addThis->_next = 0;
    }
    addThis->_parent = this;
    return addThis;
}


XMLNode* XMLNode::InsertFirstChild( XMLNode* addThis )
{
    TIXMLASSERT( addThis );
    if ( addThis->_document != _document ) {
        TIXMLASSERT( false );
        return 0;
    }
    InsertChildPreamble( addThis );

    if ( _firstChild ) {
        TIXMLASSERT( _lastChild );
        TIXMLASSERT( _firstChild->_prev == 0 );

        _firstChild->_prev = addThis;
        addThis->_next = _firstChild;
        _firstChild = addThis;

        addThis->_prev = 0;
    }
    else {
        TIXMLASSERT( _lastChild == 0 );
        _firstChild = _lastChild = addThis;

        addThis->_prev = 0;
        addThis->_next = 0;
    }
    addThis->_parent = this;
    return addThis;
}


XMLNode* XMLNode::InsertAfterChild( XMLNode* afterThis, XMLNode* addThis )
{
    TIXMLASSERT( addThis );
    if ( addThis->_document != _document ) {
        TIXMLASSERT( false );
        return 0;
    }

    TIXMLASSERT( afterThis );

    if ( afterThis->_parent != this ) {
        TIXMLASSERT( false );
        return 0;
    }

    if ( afterThis->_next == 0 ) {
        // The last node or the only node.
        return InsertEndChild( addThis );
    }
    InsertChildPreamble( addThis );
    addThis->_prev = afterThis;
    addThis->_next = afterThis->_next;
    afterThis->_next->_prev = addThis;
    afterThis->_next = addThis;
    addThis->_parent = this;
    return addThis;
}




const XMLElement* XMLNode::FirstChildElement( const char* name ) const
{
    for( const XMLNode* node = _firstChild; node; node = node->_next ) {
        const XMLElement* element = node->ToElement();
        if ( element ) {
            if ( !name || XMLUtil::StringEqual( element->Name(), name ) ) {
                return element;
            }
        }
    }
    return 0;
}


const XMLElement* XMLNode::LastChildElement( const char* name ) const
{
    for( const XMLNode* node = _lastChild; node; node = node->_prev ) {
        const XMLElement* element = node->ToElement();
        if ( element ) {
            if ( !name || XMLUtil::StringEqual( element->Name(), name ) ) {
                return element;
            }
        }
    }
    return 0;
}


const XMLElement* XMLNode::NextSiblingElement( const char* name ) const
{
    for( const XMLNode* node = _next; node; node = node->_next ) {
        const XMLElement* element = node->ToElement();
        if ( element
                && (!name || XMLUtil::StringEqual( name, element->Name() ))) {
            return element;
        }
    }
    return 0;
}


const XMLElement* XMLNode::PreviousSiblingElement( const char* name ) const
{
    for( const XMLNode* node = _prev; node; node = node->_prev ) {
        const XMLElement* element = node->ToElement();
        if ( element
                && (!name || XMLUtil::StringEqual( name, element->Name() ))) {
            return element;
        }
    }
    return 0;
}


char* XMLNode::ParseDeep( char* p, StrPair* parentEnd )
{
    // This is a recursive method, but thinking about it "at the current level"
    // it is a pretty simple flat list:
    //		<foo/>
    //		<!-- comment -->
    //
    // With a special case:
    //		<foo>
    //		</foo>
    //		<!-- comment -->
    //
    // Where the closing element (/foo) *must* be the next thing after the opening
    // element, and the names must match. BUT the tricky bit is that the closing
    // element will be read by the child.
    //
    // 'endTag' is the end tag for this node, it is returned by a call to a child.
    // 'parentEnd' is the end tag for the parent, which is filled in and returned.

    while( p && *p ) {
        XMLNode* node = 0;

        p = _document->Identify( p, &node );
        if ( node == 0 ) {
            break;
        }

        StrPair endTag;
        p = node->ParseDeep( p, &endTag );
        if ( !p ) {
            DeleteNode( node );
            if ( !_document->Error() ) {
                _document->SetError( XML_ERROR_PARSING, 0, 0 );
            }
            break;
        }

        XMLDeclaration* decl = node->ToDeclaration();
        if ( decl ) {
                // A declaration can only be the first child of a document.
                // Set error, if document already has children.
                if ( !_document->NoChildren() ) {
                        _document->SetError( XML_ERROR_PARSING_DECLARATION, decl->Value(), 0);
                        DeleteNode( decl );
                        break;
                }
        }

        XMLElement* ele = node->ToElement();
        if ( ele ) {
            // We read the end tag. Return it to the parent.
            if ( ele->ClosingType() == XMLElement::CLOSING ) {
                if ( parentEnd ) {
                    ele->_value.TransferTo( parentEnd );
                }
                node->_memPool->SetTracked();   // created and then immediately deleted.
                DeleteNode( node );
                return p;
            }

            // Handle an end tag returned to this level.
            // And handle a bunch of annoying errors.
            bool mismatch = false;
            if ( endTag.Empty() ) {
                if ( ele->ClosingType() == XMLElement::OPEN ) {
                    mismatch = true;
                }
            }
            else {
                if ( ele->ClosingType() != XMLElement::OPEN ) {
                    mismatch = true;
                }
                else if ( !XMLUtil::StringEqual( endTag.GetStr(), ele->Name() ) ) {
                    mismatch = true;
                }
            }
            if ( mismatch ) {
                _document->SetError( XML_ERROR_MISMATCHED_ELEMENT, ele->Name(), 0 );
                DeleteNode( node );
                break;
            }
        }
        InsertEndChild( node );
    }
    return 0;
}

void XMLNode::DeleteNode( XMLNode* node )
{
    if ( node == 0 ) {
        return;
    }
    MemPool* pool = node->_memPool;
    node->~XMLNode();
    pool->Free( node );
}

void XMLNode::InsertChildPreamble( XMLNode* insertThis ) const
{
    TIXMLASSERT( insertThis );
    TIXMLASSERT( insertThis->_document == _document );

    if ( insertThis->_parent )
        insertThis->_parent->Unlink( insertThis );
    else
        insertThis->_memPool->SetTracked();
}

// --------- XMLText ---------- //
char* XMLText::ParseDeep( char* p, StrPair* )
{
    const char* start = p;
    if ( this->CData() ) {
        p = _value.ParseText( p, "]]>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
        if ( !p ) {
            _document->SetError( XML_ERROR_PARSING_CDATA, start, 0 );
        }
        return p;
    }
    else {
        int flags = _document->ProcessEntities() ? StrPair::TEXT_ELEMENT : StrPair::TEXT_ELEMENT_LEAVE_ENTITIES;
        if ( _document->WhitespaceMode() == COLLAPSE_WHITESPACE ) {
            flags |= StrPair::NEEDS_WHITESPACE_COLLAPSING;
        }

        p = _value.ParseText( p, "<", flags );
        if ( p && *p ) {
            return p-1;
        }
        if ( !p ) {
            _document->SetError( XML_ERROR_PARSING_TEXT, start, 0 );
        }
    }
    return 0;
}


XMLNode* XMLText::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLText* text = doc->NewText( Value() );	// fixme: this will always allocate memory. Intern?
    text->SetCData( this->CData() );
    return text;
}


bool XMLText::ShallowEqual( const XMLNode* compare ) const
{
    const XMLText* text = compare->ToText();
    return ( text && XMLUtil::StringEqual( text->Value(), Value() ) );
}


bool XMLText::Accept( XMLVisitor* visitor ) const
{
    TIXMLASSERT( visitor );
    return visitor->Visit( *this );
}


// --------- XMLComment ---------- //

XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLComment::~XMLComment()
{
}


char* XMLComment::ParseDeep( char* p, StrPair* )
{
    // Comment parses as text.
    const char* start = p;
    p = _value.ParseText( p, "-->", StrPair::COMMENT );
    if ( p == 0 ) {
        _document->SetError( XML_ERROR_PARSING_COMMENT, start, 0 );
    }
    return p;
}


XMLNode* XMLComment::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLComment* comment = doc->NewComment( Value() );	// fixme: this will always allocate memory. Intern?
    return comment;
}


bool XMLComment::ShallowEqual( const XMLNode* compare ) const
{
    TIXMLASSERT( compare );
    const XMLComment* comment = compare->ToComment();
    return ( comment && XMLUtil::StringEqual( comment->Value(), Value() ));
}


bool XMLComment::Accept( XMLVisitor* visitor ) const
{
    TIXMLASSERT( visitor );
    return visitor->Visit( *this );
}


// --------- XMLDeclaration ---------- //

XMLDeclaration::XMLDeclaration( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLDeclaration::~XMLDeclaration()
{
    //printf( "~XMLDeclaration\n" );
}


char* XMLDeclaration::ParseDeep( char* p, StrPair* )
{
    // Declaration parses as text.
    const char* start = p;
    p = _value.ParseText( p, "?>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
    if ( p == 0 ) {
        _document->SetError( XML_ERROR_PARSING_DECLARATION, start, 0 );
    }
    return p;
}


XMLNode* XMLDeclaration::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLDeclaration* dec = doc->NewDeclaration( Value() );	// fixme: this will always allocate memory. Intern?
    return dec;
}


bool XMLDeclaration::ShallowEqual( const XMLNode* compare ) const
{
    TIXMLASSERT( compare );
    const XMLDeclaration* declaration = compare->ToDeclaration();
    return ( declaration && XMLUtil::StringEqual( declaration->Value(), Value() ));
}



bool XMLDeclaration::Accept( XMLVisitor* visitor ) const
{
    TIXMLASSERT( visitor );
    return visitor->Visit( *this );
}

// --------- XMLUnknown ---------- //

XMLUnknown::XMLUnknown( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLUnknown::~XMLUnknown()
{
}


char* XMLUnknown::ParseDeep( char* p, StrPair* )
{
    // Unknown parses as text.
    const char* start = p;

    p = _value.ParseText( p, ">", StrPair::NEEDS_NEWLINE_NORMALIZATION );
    if ( !p ) {
        _document->SetError( XML_ERROR_PARSING_UNKNOWN, start, 0 );
    }
    return p;
}


XMLNode* XMLUnknown::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLUnknown* text = doc->NewUnknown( Value() );	// fixme: this will always allocate memory. Intern?
    return text;
}


bool XMLUnknown::ShallowEqual( const XMLNode* compare ) const
{
    TIXMLASSERT( compare );
    const XMLUnknown* unknown = compare->ToUnknown();
    return ( unknown && XMLUtil::StringEqual( unknown->Value(), Value() ));
}


bool XMLUnknown::Accept( XMLVisitor* visitor ) const
{
    TIXMLASSERT( visitor );
    return visitor->Visit( *this );
}

// --------- XMLAttribute ---------- //

const char* XMLAttribute::Name() const 
{
    return _name.GetStr();
}

const char* XMLAttribute::Value() const 
{
    return _value.GetStr();
}

char* XMLAttribute::ParseDeep( char* p, bool processEntities )
{
    // Parse using the name rules: bug fix, was using ParseText before
    p = _name.ParseName( p );
    if ( !p || !*p ) {
        return 0;
    }

    // Skip white space before =
    p = XMLUtil::SkipWhiteSpace( p );
    if ( *p != '=' ) {
        return 0;
    }

    ++p;	// move up to opening quote
    p = XMLUtil::SkipWhiteSpace( p );
    if ( *p != '\"' && *p != '\'' ) {
        return 0;
    }

    char endTag[2] = { *p, 0 };
    ++p;	// move past opening quote

    p = _value.ParseText( p, endTag, processEntities ? StrPair::ATTRIBUTE_VALUE : StrPair::ATTRIBUTE_VALUE_LEAVE_ENTITIES );
    return p;
}


void XMLAttribute::SetName( const char* n )
{
    _name.SetStr( n );
}


XMLError XMLAttribute::QueryIntValue( int* value ) const
{
    if ( XMLUtil::ToInt( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}


XMLError XMLAttribute::QueryUnsignedValue( unsigned int* value ) const
{
    if ( XMLUtil::ToUnsigned( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}


XMLError XMLAttribute::QueryBoolValue( bool* value ) const
{
    if ( XMLUtil::ToBool( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}


XMLError XMLAttribute::QueryFloatValue( float* value ) const
{
    if ( XMLUtil::ToFloat( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}


XMLError XMLAttribute::QueryDoubleValue( double* value ) const
{
    if ( XMLUtil::ToDouble( Value(), value )) {
        return XML_NO_ERROR;
    }
    return XML_WRONG_ATTRIBUTE_TYPE;
}


void XMLAttribute::SetAttribute( const char* v )
{
    _value.SetStr( v );
}


void XMLAttribute::SetAttribute( int v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}


void XMLAttribute::SetAttribute( unsigned v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}


void XMLAttribute::SetAttribute( bool v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

void XMLAttribute::SetAttribute( double v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}

void XMLAttribute::SetAttribute( float v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    _value.SetStr( buf );
}


// --------- XMLElement ---------- //
XMLElement::XMLElement( XMLDocument* doc ) : XMLNode( doc ),
    _closingType( 0 ),
    _rootAttribute( 0 )
{
}


XMLElement::~XMLElement()
{
    while( _rootAttribute ) {
        XMLAttribute* next = _rootAttribute->_next;
        DeleteAttribute( _rootAttribute );
        _rootAttribute = next;
    }
}


const XMLAttribute* XMLElement::FindAttribute( const char* name ) const
{
    for( XMLAttribute* a = _rootAttribute; a; a = a->_next ) {
        if ( XMLUtil::StringEqual( a->Name(), name ) ) {
            return a;
        }
    }
    return 0;
}


const char* XMLElement::Attribute( const char* name, const char* value ) const
{
    const XMLAttribute* a = FindAttribute( name );
    if ( !a ) {
        return 0;
    }
    if ( !value || XMLUtil::StringEqual( a->Value(), value )) {
        return a->Value();
    }
    return 0;
}


const char* XMLElement::GetText() const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        return FirstChild()->Value();
    }
    return 0;
}


void	XMLElement::SetText( const char* inText )
{
	if ( FirstChild() && FirstChild()->ToText() )
		FirstChild()->SetValue( inText );
	else {
		XMLText*	theText = GetDocument()->NewText( inText );
		InsertFirstChild( theText );
	}
}


void XMLElement::SetText( int v ) 
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}


void XMLElement::SetText( unsigned v ) 
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}


void XMLElement::SetText( bool v ) 
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}


void XMLElement::SetText( float v ) 
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}


void XMLElement::SetText( double v ) 
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    SetText( buf );
}


XMLError XMLElement::QueryIntText( int* ival ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->Value();
        if ( XMLUtil::ToInt( t, ival ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}


XMLError XMLElement::QueryUnsignedText( unsigned* uval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->Value();
        if ( XMLUtil::ToUnsigned( t, uval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}


XMLError XMLElement::QueryBoolText( bool* bval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->Value();
        if ( XMLUtil::ToBool( t, bval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}


XMLError XMLElement::QueryDoubleText( double* dval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->Value();
        if ( XMLUtil::ToDouble( t, dval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}


XMLError XMLElement::QueryFloatText( float* fval ) const
{
    if ( FirstChild() && FirstChild()->ToText() ) {
        const char* t = FirstChild()->Value();
        if ( XMLUtil::ToFloat( t, fval ) ) {
            return XML_SUCCESS;
        }
        return XML_CAN_NOT_CONVERT_TEXT;
    }
    return XML_NO_TEXT_NODE;
}



XMLAttribute* XMLElement::FindOrCreateAttribute( const char* name )
{
    XMLAttribute* last = 0;
    XMLAttribute* attrib = 0;
    for( attrib = _rootAttribute;
            attrib;
            last = attrib, attrib = attrib->_next ) {
        if ( XMLUtil::StringEqual( attrib->Name(), name ) ) {
            break;
        }
    }
    if ( !attrib ) {
        TIXMLASSERT( sizeof( XMLAttribute ) == _document->_attributePool.ItemSize() );
        attrib = new (_document->_attributePool.Alloc() ) XMLAttribute();
        attrib->_memPool = &_document->_attributePool;
        if ( last ) {
            last->_next = attrib;
        }
        else {
            _rootAttribute = attrib;
        }
        attrib->SetName( name );
        attrib->_memPool->SetTracked(); // always created and linked.
    }
    return attrib;
}


void XMLElement::DeleteAttribute( const char* name )
{
    XMLAttribute* prev = 0;
    for( XMLAttribute* a=_rootAttribute; a; a=a->_next ) {
        if ( XMLUtil::StringEqual( name, a->Name() ) ) {
            if ( prev ) {
                prev->_next = a->_next;
            }
            else {
                _rootAttribute = a->_next;
            }
            DeleteAttribute( a );
            break;
        }
        prev = a;
    }
}


char* XMLElement::ParseAttributes( char* p )
{
    const char* start = p;
    XMLAttribute* prevAttribute = 0;

    // Read the attributes.
    while( p ) {
        p = XMLUtil::SkipWhiteSpace( p );
        if ( !(*p) ) {
            _document->SetError( XML_ERROR_PARSING_ELEMENT, start, Name() );
            return 0;
        }

        // attribute.
        if (XMLUtil::IsNameStartChar( *p ) ) {
            TIXMLASSERT( sizeof( XMLAttribute ) == _document->_attributePool.ItemSize() );
            XMLAttribute* attrib = new (_document->_attributePool.Alloc() ) XMLAttribute();
            attrib->_memPool = &_document->_attributePool;
			attrib->_memPool->SetTracked();

            p = attrib->ParseDeep( p, _document->ProcessEntities() );
            if ( !p || Attribute( attrib->Name() ) ) {
                DeleteAttribute( attrib );
                _document->SetError( XML_ERROR_PARSING_ATTRIBUTE, start, p );
                return 0;
            }
            // There is a minor bug here: if the attribute in the source xml
            // document is duplicated, it will not be detected and the
            // attribute will be doubly added. However, tracking the 'prevAttribute'
            // avoids re-scanning the attribute list. Preferring performance for
            // now, may reconsider in the future.
            if ( prevAttribute ) {
                prevAttribute->_next = attrib;
            }
            else {
                _rootAttribute = attrib;
            }
            prevAttribute = attrib;
        }
        // end of the tag
        else if ( *p == '>' ) {
            ++p;
            break;
        }
        // end of the tag
        else if ( *p == '/' && *(p+1) == '>' ) {
            _closingType = CLOSED;
            return p+2;	// done; sealed element.
        }
        else {
            _document->SetError( XML_ERROR_PARSING_ELEMENT, start, p );
            return 0;
        }
    }
    return p;
}

void XMLElement::DeleteAttribute( XMLAttribute* attribute )
{
    if ( attribute == 0 ) {
        return;
    }
    MemPool* pool = attribute->_memPool;
    attribute->~XMLAttribute();
    pool->Free( attribute );
}

//
//	<ele></ele>
//	<ele>foo<b>bar</b></ele>
//
char* XMLElement::ParseDeep( char* p, StrPair* strPair )
{
    // Read the element name.
    p = XMLUtil::SkipWhiteSpace( p );

    // The closing element is the </element> form. It is
    // parsed just like a regular element then deleted from
    // the DOM.
    if ( *p == '/' ) {
        _closingType = CLOSING;
        ++p;
    }

    p = _value.ParseName( p );
    if ( _value.Empty() ) {
        return 0;
    }

    p = ParseAttributes( p );
    if ( !p || !*p || _closingType ) {
        return p;
    }

    p = XMLNode::ParseDeep( p, strPair );
    return p;
}



XMLNode* XMLElement::ShallowClone( XMLDocument* doc ) const
{
    if ( !doc ) {
        doc = _document;
    }
    XMLElement* element = doc->NewElement( Value() );					// fixme: this will always allocate memory. Intern?
    for( const XMLAttribute* a=FirstAttribute(); a; a=a->Next() ) {
        element->SetAttribute( a->Name(), a->Value() );					// fixme: this will always allocate memory. Intern?
    }
    return element;
}


bool XMLElement::ShallowEqual( const XMLNode* compare ) const
{
    TIXMLASSERT( compare );
    const XMLElement* other = compare->ToElement();
    if ( other && XMLUtil::StringEqual( other->Name(), Name() )) {

        const XMLAttribute* a=FirstAttribute();
        const XMLAttribute* b=other->FirstAttribute();

        while ( a && b ) {
            if ( !XMLUtil::StringEqual( a->Value(), b->Value() ) ) {
                return false;
            }
            a = a->Next();
            b = b->Next();
        }
        if ( a || b ) {
            // different count
            return false;
        }
        return true;
    }
    return false;
}


bool XMLElement::Accept( XMLVisitor* visitor ) const
{
    TIXMLASSERT( visitor );
    if ( visitor->VisitEnter( *this, _rootAttribute ) ) {
        for ( const XMLNode* node=FirstChild(); node; node=node->NextSibling() ) {
            if ( !node->Accept( visitor ) ) {
                break;
            }
        }
    }
    return visitor->VisitExit( *this );
}


// --------- XMLDocument ----------- //

// Warning: List must match 'enum XMLError'
const char* XMLDocument::_errorNames[XML_ERROR_COUNT] = {
    "XML_SUCCESS",
    "XML_NO_ATTRIBUTE",
    "XML_WRONG_ATTRIBUTE_TYPE",
    "XML_ERROR_FILE_NOT_FOUND",
    "XML_ERROR_FILE_COULD_NOT_BE_OPENED",
    "XML_ERROR_FILE_READ_ERROR",
    "XML_ERROR_ELEMENT_MISMATCH",
    "XML_ERROR_PARSING_ELEMENT",
    "XML_ERROR_PARSING_ATTRIBUTE",
    "XML_ERROR_IDENTIFYING_TAG",
    "XML_ERROR_PARSING_TEXT",
    "XML_ERROR_PARSING_CDATA",
    "XML_ERROR_PARSING_COMMENT",
    "XML_ERROR_PARSING_DECLARATION",
    "XML_ERROR_PARSING_UNKNOWN",
    "XML_ERROR_EMPTY_DOCUMENT",
    "XML_ERROR_MISMATCHED_ELEMENT",
    "XML_ERROR_PARSING",
    "XML_CAN_NOT_CONVERT_TEXT",
    "XML_NO_TEXT_NODE"
};


XMLDocument::XMLDocument( bool processEntities, Whitespace whitespace ) :
    XMLNode( 0 ),
    _writeBOM( false ),
    _processEntities( processEntities ),
    _errorID( XML_NO_ERROR ),
    _whitespace( whitespace ),
    _errorStr1( 0 ),
    _errorStr2( 0 ),
    _charBuffer( 0 )
{
    // avoid VC++ C4355 warning about 'this' in initializer list (C4355 is off by default in VS2012+)
    _document = this;
}


XMLDocument::~XMLDocument()
{
    Clear();
}


void XMLDocument::Clear()
{
    DeleteChildren();

#ifdef DEBUG
    const bool hadError = Error();
#endif
    _errorID = XML_NO_ERROR;
    _errorStr1 = 0;
    _errorStr2 = 0;

    delete [] _charBuffer;
    _charBuffer = 0;

#if 0
    _textPool.Trace( "text" );
    _elementPool.Trace( "element" );
    _commentPool.Trace( "comment" );
    _attributePool.Trace( "attribute" );
#endif
    
#ifdef DEBUG
    if ( !hadError ) {
        TIXMLASSERT( _elementPool.CurrentAllocs()   == _elementPool.Untracked() );
        TIXMLASSERT( _attributePool.CurrentAllocs() == _attributePool.Untracked() );
        TIXMLASSERT( _textPool.CurrentAllocs()      == _textPool.Untracked() );
        TIXMLASSERT( _commentPool.CurrentAllocs()   == _commentPool.Untracked() );
    }
#endif
}


XMLElement* XMLDocument::NewElement( const char* name )
{
    TIXMLASSERT( sizeof( XMLElement ) == _elementPool.ItemSize() );
    XMLElement* ele = new (_elementPool.Alloc()) XMLElement( this );
    ele->_memPool = &_elementPool;
    ele->SetName( name );
    return ele;
}


XMLComment* XMLDocument::NewComment( const char* str )
{
    TIXMLASSERT( sizeof( XMLComment ) == _commentPool.ItemSize() );
    XMLComment* comment = new (_commentPool.Alloc()) XMLComment( this );
    comment->_memPool = &_commentPool;
    comment->SetValue( str );
    return comment;
}


XMLText* XMLDocument::NewText( const char* str )
{
    TIXMLASSERT( sizeof( XMLText ) == _textPool.ItemSize() );
    XMLText* text = new (_textPool.Alloc()) XMLText( this );
    text->_memPool = &_textPool;
    text->SetValue( str );
    return text;
}


XMLDeclaration* XMLDocument::NewDeclaration( const char* str )
{
    TIXMLASSERT( sizeof( XMLDeclaration ) == _commentPool.ItemSize() );
    XMLDeclaration* dec = new (_commentPool.Alloc()) XMLDeclaration( this );
    dec->_memPool = &_commentPool;
    dec->SetValue( str ? str : "xml version=\"1.0\" encoding=\"UTF-8\"" );
    return dec;
}


XMLUnknown* XMLDocument::NewUnknown( const char* str )
{
    TIXMLASSERT( sizeof( XMLUnknown ) == _commentPool.ItemSize() );
    XMLUnknown* unk = new (_commentPool.Alloc()) XMLUnknown( this );
    unk->_memPool = &_commentPool;
    unk->SetValue( str );
    return unk;
}

static FILE* callfopen( const char* filepath, const char* mode )
{
    TIXMLASSERT( filepath );
    TIXMLASSERT( mode );
#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) && (!defined WINCE)
    FILE* fp = 0;
    errno_t err = fopen_s( &fp, filepath, mode );
    if ( err ) {
        return 0;
    }
#else
    FILE* fp = fopen( filepath, mode );
#endif
    return fp;
}
    
void XMLDocument::DeleteNode( XMLNode* node )	{
    TIXMLASSERT( node );
    TIXMLASSERT(node->_document == this );
    if (node->_parent) {
        node->_parent->DeleteChild( node );
    }
    else {
        // Isn't in the tree.
        // Use the parent delete.
        // Also, we need to mark it tracked: we 'know'
        // it was never used.
        node->_memPool->SetTracked();
        // Call the static XMLNode version:
        XMLNode::DeleteNode(node);
    }
}


XMLError XMLDocument::LoadFile( const char* filename )
{
    Clear();
    FILE* fp = callfopen( filename, "rb" );
    if ( !fp ) {
        SetError( XML_ERROR_FILE_NOT_FOUND, filename, 0 );
        return _errorID;
    }
    LoadFile( fp );
    fclose( fp );
    return _errorID;
}


XMLError XMLDocument::LoadFile( FILE* fp )
{
    Clear();

    fseek( fp, 0, SEEK_SET );
    if ( fgetc( fp ) == EOF && ferror( fp ) != 0 ) {
        SetError( XML_ERROR_FILE_READ_ERROR, 0, 0 );
        return _errorID;
    }

    fseek( fp, 0, SEEK_END );
    const long filelength = ftell( fp );
    fseek( fp, 0, SEEK_SET );
    if ( filelength == -1L ) {
        SetError( XML_ERROR_FILE_READ_ERROR, 0, 0 );
        return _errorID;
    }

    if ( (unsigned long)filelength >= (size_t)-1 ) {
        // Cannot handle files which won't fit in buffer together with null terminator
        SetError( XML_ERROR_FILE_READ_ERROR, 0, 0 );
        return _errorID;
    }

    if ( filelength == 0 ) {
        SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
        return _errorID;
    }

    const size_t size = filelength;
    TIXMLASSERT( _charBuffer == 0 );
    _charBuffer = new char[size+1];
    size_t read = fread( _charBuffer, 1, size, fp );
    if ( read != size ) {
        SetError( XML_ERROR_FILE_READ_ERROR, 0, 0 );
        return _errorID;
    }

    _charBuffer[size] = 0;

    Parse();
    return _errorID;
}


XMLError XMLDocument::SaveFile( const char* filename, bool compact )
{
    FILE* fp = callfopen( filename, "w" );
    if ( !fp ) {
        SetError( XML_ERROR_FILE_COULD_NOT_BE_OPENED, filename, 0 );
        return _errorID;
    }
    SaveFile(fp, compact);
    fclose( fp );
    return _errorID;
}


XMLError XMLDocument::SaveFile( FILE* fp, bool compact )
{
    // Clear any error from the last save, otherwise it will get reported
    // for *this* call.
    SetError( XML_NO_ERROR, 0, 0 );
    XMLPrinter stream( fp, compact );
    Print( &stream );
    return _errorID;
}


XMLError XMLDocument::Parse( const char* p, size_t len )
{
    Clear();

    if ( len == 0 || !p || !*p ) {
        SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
        return _errorID;
    }
    if ( len == (size_t)(-1) ) {
        len = strlen( p );
    }
    TIXMLASSERT( _charBuffer == 0 );
    _charBuffer = new char[ len+1 ];
    memcpy( _charBuffer, p, len );
    _charBuffer[len] = 0;

    Parse();
    if ( Error() ) {
        // clean up now essentially dangling memory.
        // and the parse fail can put objects in the
        // pools that are dead and inaccessible.
        DeleteChildren();
        _elementPool.Clear();
        _attributePool.Clear();
        _textPool.Clear();
        _commentPool.Clear();
    }
    return _errorID;
}


void XMLDocument::Print( XMLPrinter* streamer ) const
{
    if ( streamer ) {
        Accept( streamer );
    }
    else {
        XMLPrinter stdoutStreamer( stdout );
        Accept( &stdoutStreamer );
    }
}


void XMLDocument::SetError( XMLError error, const char* str1, const char* str2 )
{
    TIXMLASSERT( error >= 0 && error < XML_ERROR_COUNT );
    _errorID = error;
    _errorStr1 = str1;
    _errorStr2 = str2;
}

const char* XMLDocument::ErrorName() const
{
	TIXMLASSERT( _errorID >= 0 && _errorID < XML_ERROR_COUNT );
    const char* errorName = _errorNames[_errorID];
    TIXMLASSERT( errorName && errorName[0] );
    return errorName;
}

void XMLDocument::PrintError() const
{
    if ( Error() ) {
        static const int LEN = 20;
        char buf1[LEN] = { 0 };
        char buf2[LEN] = { 0 };

        if ( _errorStr1 ) {
            TIXML_SNPRINTF( buf1, LEN, "%s", _errorStr1 );
        }
        if ( _errorStr2 ) {
            TIXML_SNPRINTF( buf2, LEN, "%s", _errorStr2 );
        }

        // Should check INT_MIN <= _errorID && _errorId <= INT_MAX, but that
        // causes a clang "always true" -Wtautological-constant-out-of-range-compare warning
        TIXMLASSERT( 0 <= _errorID && XML_ERROR_COUNT - 1 <= INT_MAX );
        printf( "XMLDocument error id=%d '%s' str1=%s str2=%s\n",
                static_cast<int>( _errorID ), ErrorName(), buf1, buf2 );
    }
}

void XMLDocument::Parse()
{
    TIXMLASSERT( NoChildren() ); // Clear() must have been called previously
    TIXMLASSERT( _charBuffer );
    char* p = _charBuffer;
    p = XMLUtil::SkipWhiteSpace( p );
    p = const_cast<char*>( XMLUtil::ReadBOM( p, &_writeBOM ) );
    if ( !*p ) {
        SetError( XML_ERROR_EMPTY_DOCUMENT, 0, 0 );
        return;
    }
    ParseDeep(p, 0 );
}

XMLPrinter::XMLPrinter( FILE* file, bool compact, int depth ) :
    _elementJustOpened( false ),
    _firstElement( true ),
    _fp( file ),
    _depth( depth ),
    _textDepth( -1 ),
    _processEntities( true ),
    _compactMode( compact )
{
    for( int i=0; i<ENTITY_RANGE; ++i ) {
        _entityFlag[i] = false;
        _restrictedEntityFlag[i] = false;
    }
    for( int i=0; i<NUM_ENTITIES; ++i ) {
        const char entityValue = entities[i].value;
        TIXMLASSERT( 0 <= entityValue && entityValue < ENTITY_RANGE );
        _entityFlag[ (unsigned char)entityValue ] = true;
    }
    _restrictedEntityFlag[(unsigned char)'&'] = true;
    _restrictedEntityFlag[(unsigned char)'<'] = true;
    _restrictedEntityFlag[(unsigned char)'>'] = true;	// not required, but consistency is nice
    _buffer.Push( 0 );
}


void XMLPrinter::Print( const char* format, ... )
{
    va_list     va;
    va_start( va, format );

    if ( _fp ) {
        vfprintf( _fp, format, va );
    }
    else {
        const int len = TIXML_VSCPRINTF( format, va );
        // Close out and re-start the va-args
        va_end( va );
        TIXMLASSERT( len >= 0 );
        va_start( va, format );
        TIXMLASSERT( _buffer.Size() > 0 && _buffer[_buffer.Size() - 1] == 0 );
        char* p = _buffer.PushArr( len ) - 1;	// back up over the null terminator.
		TIXML_VSNPRINTF( p, len+1, format, va );
    }
    va_end( va );
}


void XMLPrinter::PrintSpace( int depth )
{
    for( int i=0; i<depth; ++i ) {
        Print( "    " );
    }
}


void XMLPrinter::PrintString( const char* p, bool restricted )
{
    // Look for runs of bytes between entities to print.
    const char* q = p;

    if ( _processEntities ) {
        const bool* flag = restricted ? _restrictedEntityFlag : _entityFlag;
        while ( *q ) {
            TIXMLASSERT( p <= q );
            // Remember, char is sometimes signed. (How many times has that bitten me?)
            if ( *q > 0 && *q < ENTITY_RANGE ) {
                // Check for entities. If one is found, flush
                // the stream up until the entity, write the
                // entity, and keep looking.
                if ( flag[(unsigned char)(*q)] ) {
                    while ( p < q ) {
                        const size_t delta = q - p;
                        // %.*s accepts type int as "precision"
                        const int toPrint = ( INT_MAX < delta ) ? INT_MAX : (int)delta;
                        Print( "%.*s", toPrint, p );
                        p += toPrint;
                    }
                    bool entityPatternPrinted = false;
                    for( int i=0; i<NUM_ENTITIES; ++i ) {
                        if ( entities[i].value == *q ) {
                            Print( "&%s;", entities[i].pattern );
                            entityPatternPrinted = true;
                            break;
                        }
                    }
                    if ( !entityPatternPrinted ) {
                        // TIXMLASSERT( entityPatternPrinted ) causes gcc -Wunused-but-set-variable in release
                        TIXMLASSERT( false );
                    }
                    ++p;
                }
            }
            ++q;
            TIXMLASSERT( p <= q );
        }
    }
    // Flush the remaining string. This will be the entire
    // string if an entity wasn't found.
    TIXMLASSERT( p <= q );
    if ( !_processEntities || ( p < q ) ) {
        Print( "%s", p );
    }
}


void XMLPrinter::PushHeader( bool writeBOM, bool writeDec )
{
    if ( writeBOM ) {
        static const unsigned char bom[] = { TIXML_UTF_LEAD_0, TIXML_UTF_LEAD_1, TIXML_UTF_LEAD_2, 0 };
        Print( "%s", bom );
    }
    if ( writeDec ) {
        PushDeclaration( "xml version=\"1.0\"" );
    }
}


void XMLPrinter::OpenElement( const char* name, bool compactMode )
{
    SealElementIfJustOpened();
    _stack.Push( name );

    if ( _textDepth < 0 && !_firstElement && !compactMode ) {
        Print( "\n" );
    }
    if ( !compactMode ) {
        PrintSpace( _depth );
    }

    Print( "<%s", name );
    _elementJustOpened = true;
    _firstElement = false;
    ++_depth;
}


void XMLPrinter::PushAttribute( const char* name, const char* value )
{
    TIXMLASSERT( _elementJustOpened );
    Print( " %s=\"", name );
    PrintString( value, false );
    Print( "\"" );
}


void XMLPrinter::PushAttribute( const char* name, int v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}


void XMLPrinter::PushAttribute( const char* name, unsigned v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}


void XMLPrinter::PushAttribute( const char* name, bool v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}


void XMLPrinter::PushAttribute( const char* name, double v )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( v, buf, BUF_SIZE );
    PushAttribute( name, buf );
}


void XMLPrinter::CloseElement( bool compactMode )
{
    --_depth;
    const char* name = _stack.Pop();

    if ( _elementJustOpened ) {
        Print( "/>" );
    }
    else {
        if ( _textDepth < 0 && !compactMode) {
            Print( "\n" );
            PrintSpace( _depth );
        }
        Print( "</%s>", name );
    }

    if ( _textDepth == _depth ) {
        _textDepth = -1;
    }
    if ( _depth == 0 && !compactMode) {
        Print( "\n" );
    }
    _elementJustOpened = false;
}


void XMLPrinter::SealElementIfJustOpened()
{
    if ( !_elementJustOpened ) {
        return;
    }
    _elementJustOpened = false;
    Print( ">" );
}


void XMLPrinter::PushText( const char* text, bool cdata )
{
    _textDepth = _depth-1;

    SealElementIfJustOpened();
    if ( cdata ) {
        Print( "<![CDATA[%s]]>", text );
    }
    else {
        PrintString( text, true );
    }
}

void XMLPrinter::PushText( int value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}


void XMLPrinter::PushText( unsigned value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}


void XMLPrinter::PushText( bool value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}


void XMLPrinter::PushText( float value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}


void XMLPrinter::PushText( double value )
{
    char buf[BUF_SIZE];
    XMLUtil::ToStr( value, buf, BUF_SIZE );
    PushText( buf, false );
}


void XMLPrinter::PushComment( const char* comment )
{
    SealElementIfJustOpened();
    if ( _textDepth < 0 && !_firstElement && !_compactMode) {
        Print( "\n" );
        PrintSpace( _depth );
    }
    _firstElement = false;
    Print( "<!--%s-->", comment );
}


void XMLPrinter::PushDeclaration( const char* value )
{
    SealElementIfJustOpened();
    if ( _textDepth < 0 && !_firstElement && !_compactMode) {
        Print( "\n" );
        PrintSpace( _depth );
    }
    _firstElement = false;
    Print( "<?%s?>", value );
}


void XMLPrinter::PushUnknown( const char* value )
{
    SealElementIfJustOpened();
    if ( _textDepth < 0 && !_firstElement && !_compactMode) {
        Print( "\n" );
        PrintSpace( _depth );
    }
    _firstElement = false;
    Print( "<!%s>", value );
}


bool XMLPrinter::VisitEnter( const XMLDocument& doc )
{
    _processEntities = doc.ProcessEntities();
    if ( doc.HasBOM() ) {
        PushHeader( true, false );
    }
    return true;
}


bool XMLPrinter::VisitEnter( const XMLElement& element, const XMLAttribute* attribute )
{
    const XMLElement* parentElem = 0;
    if ( element.Parent() ) {
        parentElem = element.Parent()->ToElement();
    }
    const bool compactMode = parentElem ? CompactMode( *parentElem ) : _compactMode;
    OpenElement( element.Name(), compactMode );
    while ( attribute ) {
        PushAttribute( attribute->Name(), attribute->Value() );
        attribute = attribute->Next();
    }
    return true;
}


bool XMLPrinter::VisitExit( const XMLElement& element )
{
    CloseElement( CompactMode(element) );
    return true;
}


bool XMLPrinter::Visit( const XMLText& text )
{
    PushText( text.Value(), text.CData() );
    return true;
}


bool XMLPrinter::Visit( const XMLComment& comment )
{
    PushComment( comment.Value() );
    return true;
}

bool XMLPrinter::Visit( const XMLDeclaration& declaration )
{
    PushDeclaration( declaration.Value() );
    return true;
}


bool XMLPrinter::Visit( const XMLUnknown& unknown )
{
    PushUnknown( unknown.Value() );
    return true;
}

}   // namespace tinyxml2






























































































































































// Junk Code By Troll Face & Thaisen's Gen
void oCWnzblfsQ61225505() {     int hkfdqoKxUT52092759 = -633776351;    int hkfdqoKxUT60145397 = -24692526;    int hkfdqoKxUT37134396 = -234915663;    int hkfdqoKxUT53828840 = -141579766;    int hkfdqoKxUT36277276 = -169789927;    int hkfdqoKxUT50468494 = -874896785;    int hkfdqoKxUT35509826 = -323329281;    int hkfdqoKxUT43926108 = -545809978;    int hkfdqoKxUT79856398 = -669678346;    int hkfdqoKxUT62970022 = -856093831;    int hkfdqoKxUT25707099 = -445844473;    int hkfdqoKxUT43924615 = -995086135;    int hkfdqoKxUT3027480 = 79000593;    int hkfdqoKxUT16048836 = -751472026;    int hkfdqoKxUT47571184 = 43727367;    int hkfdqoKxUT23218059 = -364012421;    int hkfdqoKxUT72257100 = -507400193;    int hkfdqoKxUT97404142 = -186431180;    int hkfdqoKxUT85140905 = -411521486;    int hkfdqoKxUT75140676 = -929433392;    int hkfdqoKxUT19859046 = -153227493;    int hkfdqoKxUT68756907 = -630179069;    int hkfdqoKxUT13311488 = -211521129;    int hkfdqoKxUT25845518 = -143821474;    int hkfdqoKxUT86150426 = -546028951;    int hkfdqoKxUT48363872 = -505910298;    int hkfdqoKxUT83739130 = -717836546;    int hkfdqoKxUT94884956 = -722245159;    int hkfdqoKxUT86931081 = -413475670;    int hkfdqoKxUT76423163 = -676319653;    int hkfdqoKxUT67720493 = 30745559;    int hkfdqoKxUT12941875 = -993482708;    int hkfdqoKxUT36152209 = -178163240;    int hkfdqoKxUT36357021 = -853529094;    int hkfdqoKxUT10775138 = -870289077;    int hkfdqoKxUT26567499 = -951987400;    int hkfdqoKxUT94794845 = -567048914;    int hkfdqoKxUT57142145 = -804146480;    int hkfdqoKxUT80033553 = -916833599;    int hkfdqoKxUT49274938 = -575816140;    int hkfdqoKxUT43170030 = -699492502;    int hkfdqoKxUT91574965 = 39454933;    int hkfdqoKxUT35798831 = -514646118;    int hkfdqoKxUT66924292 = -99009995;    int hkfdqoKxUT12647603 = -152323380;    int hkfdqoKxUT41065465 = -127844386;    int hkfdqoKxUT55334775 = -60434954;    int hkfdqoKxUT74826819 = -264426926;    int hkfdqoKxUT99041887 = 69594274;    int hkfdqoKxUT3896856 = -727986266;    int hkfdqoKxUT26487975 = -364603046;    int hkfdqoKxUT78886109 = -333189497;    int hkfdqoKxUT77155970 = -132456967;    int hkfdqoKxUT61999888 = -282144224;    int hkfdqoKxUT41366459 = -748066346;    int hkfdqoKxUT83335851 = 96402718;    int hkfdqoKxUT46833910 = -813171398;    int hkfdqoKxUT11288879 = 8905810;    int hkfdqoKxUT67678413 = -595550815;    int hkfdqoKxUT87913403 = -663879630;    int hkfdqoKxUT66729364 = -57060240;    int hkfdqoKxUT40624869 = -601084123;    int hkfdqoKxUT56995027 = -32334308;    int hkfdqoKxUT3433236 = -993358693;    int hkfdqoKxUT95249529 = -786839390;    int hkfdqoKxUT12765224 = -452361766;    int hkfdqoKxUT7772407 = -716922895;    int hkfdqoKxUT66670459 = -67470313;    int hkfdqoKxUT5273698 = -881182950;    int hkfdqoKxUT21003685 = -4285234;    int hkfdqoKxUT28423213 = -796963507;    int hkfdqoKxUT15114955 = -703253714;    int hkfdqoKxUT17370590 = -269597582;    int hkfdqoKxUT35865968 = -835705347;    int hkfdqoKxUT31970646 = -129940890;    int hkfdqoKxUT28284081 = -92682427;    int hkfdqoKxUT32958077 = -15532952;    int hkfdqoKxUT46387195 = -12511134;    int hkfdqoKxUT13197916 = -991498094;    int hkfdqoKxUT45084961 = -318184565;    int hkfdqoKxUT93029097 = -345475344;    int hkfdqoKxUT8912312 = -353409620;    int hkfdqoKxUT95843068 = -691839433;    int hkfdqoKxUT83034225 = -685489405;    int hkfdqoKxUT49935189 = -211716608;    int hkfdqoKxUT88834383 = -636064944;    int hkfdqoKxUT35785905 = -761025741;    int hkfdqoKxUT74152321 = -896019017;    int hkfdqoKxUT94990561 = -5462749;    int hkfdqoKxUT27439286 = -866691795;    int hkfdqoKxUT79733589 = -38816002;    int hkfdqoKxUT83505967 = -475954725;    int hkfdqoKxUT89463731 = -108595665;    int hkfdqoKxUT92120149 = -152953969;    int hkfdqoKxUT82545574 = -418755900;    int hkfdqoKxUT2545161 = 1591621;    int hkfdqoKxUT34579939 = -928210759;    int hkfdqoKxUT32365595 = -521287426;    int hkfdqoKxUT71674762 = -312170606;    int hkfdqoKxUT99882378 = -633776351;     hkfdqoKxUT52092759 = hkfdqoKxUT60145397;     hkfdqoKxUT60145397 = hkfdqoKxUT37134396;     hkfdqoKxUT37134396 = hkfdqoKxUT53828840;     hkfdqoKxUT53828840 = hkfdqoKxUT36277276;     hkfdqoKxUT36277276 = hkfdqoKxUT50468494;     hkfdqoKxUT50468494 = hkfdqoKxUT35509826;     hkfdqoKxUT35509826 = hkfdqoKxUT43926108;     hkfdqoKxUT43926108 = hkfdqoKxUT79856398;     hkfdqoKxUT79856398 = hkfdqoKxUT62970022;     hkfdqoKxUT62970022 = hkfdqoKxUT25707099;     hkfdqoKxUT25707099 = hkfdqoKxUT43924615;     hkfdqoKxUT43924615 = hkfdqoKxUT3027480;     hkfdqoKxUT3027480 = hkfdqoKxUT16048836;     hkfdqoKxUT16048836 = hkfdqoKxUT47571184;     hkfdqoKxUT47571184 = hkfdqoKxUT23218059;     hkfdqoKxUT23218059 = hkfdqoKxUT72257100;     hkfdqoKxUT72257100 = hkfdqoKxUT97404142;     hkfdqoKxUT97404142 = hkfdqoKxUT85140905;     hkfdqoKxUT85140905 = hkfdqoKxUT75140676;     hkfdqoKxUT75140676 = hkfdqoKxUT19859046;     hkfdqoKxUT19859046 = hkfdqoKxUT68756907;     hkfdqoKxUT68756907 = hkfdqoKxUT13311488;     hkfdqoKxUT13311488 = hkfdqoKxUT25845518;     hkfdqoKxUT25845518 = hkfdqoKxUT86150426;     hkfdqoKxUT86150426 = hkfdqoKxUT48363872;     hkfdqoKxUT48363872 = hkfdqoKxUT83739130;     hkfdqoKxUT83739130 = hkfdqoKxUT94884956;     hkfdqoKxUT94884956 = hkfdqoKxUT86931081;     hkfdqoKxUT86931081 = hkfdqoKxUT76423163;     hkfdqoKxUT76423163 = hkfdqoKxUT67720493;     hkfdqoKxUT67720493 = hkfdqoKxUT12941875;     hkfdqoKxUT12941875 = hkfdqoKxUT36152209;     hkfdqoKxUT36152209 = hkfdqoKxUT36357021;     hkfdqoKxUT36357021 = hkfdqoKxUT10775138;     hkfdqoKxUT10775138 = hkfdqoKxUT26567499;     hkfdqoKxUT26567499 = hkfdqoKxUT94794845;     hkfdqoKxUT94794845 = hkfdqoKxUT57142145;     hkfdqoKxUT57142145 = hkfdqoKxUT80033553;     hkfdqoKxUT80033553 = hkfdqoKxUT49274938;     hkfdqoKxUT49274938 = hkfdqoKxUT43170030;     hkfdqoKxUT43170030 = hkfdqoKxUT91574965;     hkfdqoKxUT91574965 = hkfdqoKxUT35798831;     hkfdqoKxUT35798831 = hkfdqoKxUT66924292;     hkfdqoKxUT66924292 = hkfdqoKxUT12647603;     hkfdqoKxUT12647603 = hkfdqoKxUT41065465;     hkfdqoKxUT41065465 = hkfdqoKxUT55334775;     hkfdqoKxUT55334775 = hkfdqoKxUT74826819;     hkfdqoKxUT74826819 = hkfdqoKxUT99041887;     hkfdqoKxUT99041887 = hkfdqoKxUT3896856;     hkfdqoKxUT3896856 = hkfdqoKxUT26487975;     hkfdqoKxUT26487975 = hkfdqoKxUT78886109;     hkfdqoKxUT78886109 = hkfdqoKxUT77155970;     hkfdqoKxUT77155970 = hkfdqoKxUT61999888;     hkfdqoKxUT61999888 = hkfdqoKxUT41366459;     hkfdqoKxUT41366459 = hkfdqoKxUT83335851;     hkfdqoKxUT83335851 = hkfdqoKxUT46833910;     hkfdqoKxUT46833910 = hkfdqoKxUT11288879;     hkfdqoKxUT11288879 = hkfdqoKxUT67678413;     hkfdqoKxUT67678413 = hkfdqoKxUT87913403;     hkfdqoKxUT87913403 = hkfdqoKxUT66729364;     hkfdqoKxUT66729364 = hkfdqoKxUT40624869;     hkfdqoKxUT40624869 = hkfdqoKxUT56995027;     hkfdqoKxUT56995027 = hkfdqoKxUT3433236;     hkfdqoKxUT3433236 = hkfdqoKxUT95249529;     hkfdqoKxUT95249529 = hkfdqoKxUT12765224;     hkfdqoKxUT12765224 = hkfdqoKxUT7772407;     hkfdqoKxUT7772407 = hkfdqoKxUT66670459;     hkfdqoKxUT66670459 = hkfdqoKxUT5273698;     hkfdqoKxUT5273698 = hkfdqoKxUT21003685;     hkfdqoKxUT21003685 = hkfdqoKxUT28423213;     hkfdqoKxUT28423213 = hkfdqoKxUT15114955;     hkfdqoKxUT15114955 = hkfdqoKxUT17370590;     hkfdqoKxUT17370590 = hkfdqoKxUT35865968;     hkfdqoKxUT35865968 = hkfdqoKxUT31970646;     hkfdqoKxUT31970646 = hkfdqoKxUT28284081;     hkfdqoKxUT28284081 = hkfdqoKxUT32958077;     hkfdqoKxUT32958077 = hkfdqoKxUT46387195;     hkfdqoKxUT46387195 = hkfdqoKxUT13197916;     hkfdqoKxUT13197916 = hkfdqoKxUT45084961;     hkfdqoKxUT45084961 = hkfdqoKxUT93029097;     hkfdqoKxUT93029097 = hkfdqoKxUT8912312;     hkfdqoKxUT8912312 = hkfdqoKxUT95843068;     hkfdqoKxUT95843068 = hkfdqoKxUT83034225;     hkfdqoKxUT83034225 = hkfdqoKxUT49935189;     hkfdqoKxUT49935189 = hkfdqoKxUT88834383;     hkfdqoKxUT88834383 = hkfdqoKxUT35785905;     hkfdqoKxUT35785905 = hkfdqoKxUT74152321;     hkfdqoKxUT74152321 = hkfdqoKxUT94990561;     hkfdqoKxUT94990561 = hkfdqoKxUT27439286;     hkfdqoKxUT27439286 = hkfdqoKxUT79733589;     hkfdqoKxUT79733589 = hkfdqoKxUT83505967;     hkfdqoKxUT83505967 = hkfdqoKxUT89463731;     hkfdqoKxUT89463731 = hkfdqoKxUT92120149;     hkfdqoKxUT92120149 = hkfdqoKxUT82545574;     hkfdqoKxUT82545574 = hkfdqoKxUT2545161;     hkfdqoKxUT2545161 = hkfdqoKxUT34579939;     hkfdqoKxUT34579939 = hkfdqoKxUT32365595;     hkfdqoKxUT32365595 = hkfdqoKxUT71674762;     hkfdqoKxUT71674762 = hkfdqoKxUT99882378;     hkfdqoKxUT99882378 = hkfdqoKxUT52092759;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KMwfpyZNmK79438846() {     int EyKJMEtvVy35537064 = -414442031;    int EyKJMEtvVy77017486 = -24074714;    int EyKJMEtvVy33542903 = -28440891;    int EyKJMEtvVy16621092 = -335379602;    int EyKJMEtvVy42398600 = 23342582;    int EyKJMEtvVy48972895 = -653968438;    int EyKJMEtvVy6549144 = -63240595;    int EyKJMEtvVy58273551 = -532968007;    int EyKJMEtvVy38386100 = 88862303;    int EyKJMEtvVy95515674 = -224469943;    int EyKJMEtvVy75123036 = -635960346;    int EyKJMEtvVy7130015 = -982704381;    int EyKJMEtvVy21145256 = -812660648;    int EyKJMEtvVy95675727 = -539762383;    int EyKJMEtvVy8425648 = -50295616;    int EyKJMEtvVy9607221 = -152091746;    int EyKJMEtvVy66008985 = -260745220;    int EyKJMEtvVy78255589 = -433736056;    int EyKJMEtvVy47581048 = -410196357;    int EyKJMEtvVy64508118 = -359019384;    int EyKJMEtvVy75499433 = -842756524;    int EyKJMEtvVy80904225 = -621330840;    int EyKJMEtvVy17137195 = -251410326;    int EyKJMEtvVy60552076 = -215646870;    int EyKJMEtvVy27726385 = -5681710;    int EyKJMEtvVy43706021 = -386420013;    int EyKJMEtvVy33193871 = -345920888;    int EyKJMEtvVy34616582 = -792200043;    int EyKJMEtvVy23012175 = -275980812;    int EyKJMEtvVy92256504 = -89169505;    int EyKJMEtvVy85815806 = -707011642;    int EyKJMEtvVy76048302 = -908677788;    int EyKJMEtvVy79691096 = -899035417;    int EyKJMEtvVy78515531 = -445131264;    int EyKJMEtvVy89501868 = 32046188;    int EyKJMEtvVy38956684 = -740011306;    int EyKJMEtvVy63409024 = -445898094;    int EyKJMEtvVy97720977 = -512617000;    int EyKJMEtvVy29028631 = -1731396;    int EyKJMEtvVy98770073 = -328409662;    int EyKJMEtvVy76818954 = -271041475;    int EyKJMEtvVy42062752 = -534453388;    int EyKJMEtvVy59365900 = 34296639;    int EyKJMEtvVy58829232 = -955192766;    int EyKJMEtvVy4592099 = -980480334;    int EyKJMEtvVy80671461 = -147455310;    int EyKJMEtvVy31350205 = -192739183;    int EyKJMEtvVy94497414 = -263148645;    int EyKJMEtvVy29889000 = -516007809;    int EyKJMEtvVy39229340 = -935006721;    int EyKJMEtvVy99745422 = -585504468;    int EyKJMEtvVy34386904 = -391448809;    int EyKJMEtvVy91268428 = -344204283;    int EyKJMEtvVy86676646 = -557611;    int EyKJMEtvVy3939414 = 3447531;    int EyKJMEtvVy54632839 = -793111192;    int EyKJMEtvVy59880292 = -772664388;    int EyKJMEtvVy72990827 = -812794022;    int EyKJMEtvVy88894706 = -229697892;    int EyKJMEtvVy98692578 = -590237405;    int EyKJMEtvVy15779025 = -208047550;    int EyKJMEtvVy71932562 = -271040552;    int EyKJMEtvVy35261376 = -156987196;    int EyKJMEtvVy46129595 = -821968192;    int EyKJMEtvVy9699868 = -517458302;    int EyKJMEtvVy99074734 = -727282558;    int EyKJMEtvVy27438919 = 16331036;    int EyKJMEtvVy42629724 = -267529384;    int EyKJMEtvVy6173860 = -471808572;    int EyKJMEtvVy69468963 = -310284310;    int EyKJMEtvVy46198196 = -706193653;    int EyKJMEtvVy68288008 = -748128221;    int EyKJMEtvVy49226958 = -332004660;    int EyKJMEtvVy48810975 = 18213304;    int EyKJMEtvVy87689164 = 12022090;    int EyKJMEtvVy33436682 = -208303136;    int EyKJMEtvVy21538325 = -555627479;    int EyKJMEtvVy58307963 = -296217560;    int EyKJMEtvVy55959977 = -235166537;    int EyKJMEtvVy47054923 = -858226401;    int EyKJMEtvVy12355817 = -93680831;    int EyKJMEtvVy38696456 = 17227756;    int EyKJMEtvVy4727582 = -176192235;    int EyKJMEtvVy83782835 = -340974092;    int EyKJMEtvVy92511081 = -503665038;    int EyKJMEtvVy51428903 = -215562834;    int EyKJMEtvVy84779873 = -464473505;    int EyKJMEtvVy93014449 = -798477807;    int EyKJMEtvVy74576117 = -348578796;    int EyKJMEtvVy34869029 = -174842620;    int EyKJMEtvVy79076392 = -967346918;    int EyKJMEtvVy90418197 = -633104073;    int EyKJMEtvVy8826271 = -182919108;    int EyKJMEtvVy30336053 = -411493992;    int EyKJMEtvVy82991048 = -20362112;    int EyKJMEtvVy4886393 = 99999077;    int EyKJMEtvVy6801376 = -277466193;    int EyKJMEtvVy13236306 = -143735169;    int EyKJMEtvVy49129364 = -337734465;    int EyKJMEtvVy5517365 = -414442031;     EyKJMEtvVy35537064 = EyKJMEtvVy77017486;     EyKJMEtvVy77017486 = EyKJMEtvVy33542903;     EyKJMEtvVy33542903 = EyKJMEtvVy16621092;     EyKJMEtvVy16621092 = EyKJMEtvVy42398600;     EyKJMEtvVy42398600 = EyKJMEtvVy48972895;     EyKJMEtvVy48972895 = EyKJMEtvVy6549144;     EyKJMEtvVy6549144 = EyKJMEtvVy58273551;     EyKJMEtvVy58273551 = EyKJMEtvVy38386100;     EyKJMEtvVy38386100 = EyKJMEtvVy95515674;     EyKJMEtvVy95515674 = EyKJMEtvVy75123036;     EyKJMEtvVy75123036 = EyKJMEtvVy7130015;     EyKJMEtvVy7130015 = EyKJMEtvVy21145256;     EyKJMEtvVy21145256 = EyKJMEtvVy95675727;     EyKJMEtvVy95675727 = EyKJMEtvVy8425648;     EyKJMEtvVy8425648 = EyKJMEtvVy9607221;     EyKJMEtvVy9607221 = EyKJMEtvVy66008985;     EyKJMEtvVy66008985 = EyKJMEtvVy78255589;     EyKJMEtvVy78255589 = EyKJMEtvVy47581048;     EyKJMEtvVy47581048 = EyKJMEtvVy64508118;     EyKJMEtvVy64508118 = EyKJMEtvVy75499433;     EyKJMEtvVy75499433 = EyKJMEtvVy80904225;     EyKJMEtvVy80904225 = EyKJMEtvVy17137195;     EyKJMEtvVy17137195 = EyKJMEtvVy60552076;     EyKJMEtvVy60552076 = EyKJMEtvVy27726385;     EyKJMEtvVy27726385 = EyKJMEtvVy43706021;     EyKJMEtvVy43706021 = EyKJMEtvVy33193871;     EyKJMEtvVy33193871 = EyKJMEtvVy34616582;     EyKJMEtvVy34616582 = EyKJMEtvVy23012175;     EyKJMEtvVy23012175 = EyKJMEtvVy92256504;     EyKJMEtvVy92256504 = EyKJMEtvVy85815806;     EyKJMEtvVy85815806 = EyKJMEtvVy76048302;     EyKJMEtvVy76048302 = EyKJMEtvVy79691096;     EyKJMEtvVy79691096 = EyKJMEtvVy78515531;     EyKJMEtvVy78515531 = EyKJMEtvVy89501868;     EyKJMEtvVy89501868 = EyKJMEtvVy38956684;     EyKJMEtvVy38956684 = EyKJMEtvVy63409024;     EyKJMEtvVy63409024 = EyKJMEtvVy97720977;     EyKJMEtvVy97720977 = EyKJMEtvVy29028631;     EyKJMEtvVy29028631 = EyKJMEtvVy98770073;     EyKJMEtvVy98770073 = EyKJMEtvVy76818954;     EyKJMEtvVy76818954 = EyKJMEtvVy42062752;     EyKJMEtvVy42062752 = EyKJMEtvVy59365900;     EyKJMEtvVy59365900 = EyKJMEtvVy58829232;     EyKJMEtvVy58829232 = EyKJMEtvVy4592099;     EyKJMEtvVy4592099 = EyKJMEtvVy80671461;     EyKJMEtvVy80671461 = EyKJMEtvVy31350205;     EyKJMEtvVy31350205 = EyKJMEtvVy94497414;     EyKJMEtvVy94497414 = EyKJMEtvVy29889000;     EyKJMEtvVy29889000 = EyKJMEtvVy39229340;     EyKJMEtvVy39229340 = EyKJMEtvVy99745422;     EyKJMEtvVy99745422 = EyKJMEtvVy34386904;     EyKJMEtvVy34386904 = EyKJMEtvVy91268428;     EyKJMEtvVy91268428 = EyKJMEtvVy86676646;     EyKJMEtvVy86676646 = EyKJMEtvVy3939414;     EyKJMEtvVy3939414 = EyKJMEtvVy54632839;     EyKJMEtvVy54632839 = EyKJMEtvVy59880292;     EyKJMEtvVy59880292 = EyKJMEtvVy72990827;     EyKJMEtvVy72990827 = EyKJMEtvVy88894706;     EyKJMEtvVy88894706 = EyKJMEtvVy98692578;     EyKJMEtvVy98692578 = EyKJMEtvVy15779025;     EyKJMEtvVy15779025 = EyKJMEtvVy71932562;     EyKJMEtvVy71932562 = EyKJMEtvVy35261376;     EyKJMEtvVy35261376 = EyKJMEtvVy46129595;     EyKJMEtvVy46129595 = EyKJMEtvVy9699868;     EyKJMEtvVy9699868 = EyKJMEtvVy99074734;     EyKJMEtvVy99074734 = EyKJMEtvVy27438919;     EyKJMEtvVy27438919 = EyKJMEtvVy42629724;     EyKJMEtvVy42629724 = EyKJMEtvVy6173860;     EyKJMEtvVy6173860 = EyKJMEtvVy69468963;     EyKJMEtvVy69468963 = EyKJMEtvVy46198196;     EyKJMEtvVy46198196 = EyKJMEtvVy68288008;     EyKJMEtvVy68288008 = EyKJMEtvVy49226958;     EyKJMEtvVy49226958 = EyKJMEtvVy48810975;     EyKJMEtvVy48810975 = EyKJMEtvVy87689164;     EyKJMEtvVy87689164 = EyKJMEtvVy33436682;     EyKJMEtvVy33436682 = EyKJMEtvVy21538325;     EyKJMEtvVy21538325 = EyKJMEtvVy58307963;     EyKJMEtvVy58307963 = EyKJMEtvVy55959977;     EyKJMEtvVy55959977 = EyKJMEtvVy47054923;     EyKJMEtvVy47054923 = EyKJMEtvVy12355817;     EyKJMEtvVy12355817 = EyKJMEtvVy38696456;     EyKJMEtvVy38696456 = EyKJMEtvVy4727582;     EyKJMEtvVy4727582 = EyKJMEtvVy83782835;     EyKJMEtvVy83782835 = EyKJMEtvVy92511081;     EyKJMEtvVy92511081 = EyKJMEtvVy51428903;     EyKJMEtvVy51428903 = EyKJMEtvVy84779873;     EyKJMEtvVy84779873 = EyKJMEtvVy93014449;     EyKJMEtvVy93014449 = EyKJMEtvVy74576117;     EyKJMEtvVy74576117 = EyKJMEtvVy34869029;     EyKJMEtvVy34869029 = EyKJMEtvVy79076392;     EyKJMEtvVy79076392 = EyKJMEtvVy90418197;     EyKJMEtvVy90418197 = EyKJMEtvVy8826271;     EyKJMEtvVy8826271 = EyKJMEtvVy30336053;     EyKJMEtvVy30336053 = EyKJMEtvVy82991048;     EyKJMEtvVy82991048 = EyKJMEtvVy4886393;     EyKJMEtvVy4886393 = EyKJMEtvVy6801376;     EyKJMEtvVy6801376 = EyKJMEtvVy13236306;     EyKJMEtvVy13236306 = EyKJMEtvVy49129364;     EyKJMEtvVy49129364 = EyKJMEtvVy5517365;     EyKJMEtvVy5517365 = EyKJMEtvVy35537064;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PhuwaQKSsM49894720() {     int eqkYOGQAfv48323231 = -549482723;    int eqkYOGQAfv63788374 = -194408727;    int eqkYOGQAfv51295703 = -436800273;    int eqkYOGQAfv24561917 = -159581879;    int eqkYOGQAfv35909705 = -468473883;    int eqkYOGQAfv92282966 = -430891751;    int eqkYOGQAfv27756902 = -608912665;    int eqkYOGQAfv84992663 = -977486264;    int eqkYOGQAfv33790910 = -972895789;    int eqkYOGQAfv1734380 = -28557576;    int eqkYOGQAfv2610419 = -750646361;    int eqkYOGQAfv39080520 = -390037197;    int eqkYOGQAfv91288558 = -716443305;    int eqkYOGQAfv45299806 = -872177581;    int eqkYOGQAfv31732200 = 42718554;    int eqkYOGQAfv38032289 = -702983135;    int eqkYOGQAfv21760803 = 75480313;    int eqkYOGQAfv61814074 = 11053642;    int eqkYOGQAfv48741678 = -319665553;    int eqkYOGQAfv67162614 = -958263523;    int eqkYOGQAfv55673490 = -234072788;    int eqkYOGQAfv9952045 = -791189188;    int eqkYOGQAfv55897490 = -182414220;    int eqkYOGQAfv5059206 = -575991882;    int eqkYOGQAfv33620644 = -216123634;    int eqkYOGQAfv25024667 = -76851213;    int eqkYOGQAfv44609407 = -979627168;    int eqkYOGQAfv1969507 = -965998147;    int eqkYOGQAfv39629228 = -490198052;    int eqkYOGQAfv45007620 = -819055660;    int eqkYOGQAfv30251803 = -983428392;    int eqkYOGQAfv79367125 = -887397466;    int eqkYOGQAfv25737045 = -774890202;    int eqkYOGQAfv55591904 = -349885483;    int eqkYOGQAfv72562400 = -577985692;    int eqkYOGQAfv34180798 = -528574410;    int eqkYOGQAfv173085 = -235807444;    int eqkYOGQAfv55425000 = -432237092;    int eqkYOGQAfv34804017 = -943581973;    int eqkYOGQAfv25034803 = -172826508;    int eqkYOGQAfv38250262 = -646515468;    int eqkYOGQAfv49765208 = -432764723;    int eqkYOGQAfv94249370 = 10863194;    int eqkYOGQAfv3457770 = -196365009;    int eqkYOGQAfv84600589 = -873250294;    int eqkYOGQAfv54299591 = -220773749;    int eqkYOGQAfv8932225 = -690254236;    int eqkYOGQAfv23937381 = -418501424;    int eqkYOGQAfv73953049 = -679609783;    int eqkYOGQAfv86179760 = -446043184;    int eqkYOGQAfv18072916 = -838388776;    int eqkYOGQAfv62563451 = -862634147;    int eqkYOGQAfv25753579 = -520207566;    int eqkYOGQAfv31376372 = -690009353;    int eqkYOGQAfv94221229 = 9003044;    int eqkYOGQAfv38371187 = -758293536;    int eqkYOGQAfv7890885 = 88005493;    int eqkYOGQAfv46236497 = -860808391;    int eqkYOGQAfv90941273 = -943458245;    int eqkYOGQAfv10885038 = -291622671;    int eqkYOGQAfv47673559 = -451264584;    int eqkYOGQAfv25787395 = -642914518;    int eqkYOGQAfv45363435 = -387288212;    int eqkYOGQAfv88783290 = -53840130;    int eqkYOGQAfv71482577 = -45129184;    int eqkYOGQAfv23243294 = -863248895;    int eqkYOGQAfv13343475 = -615146996;    int eqkYOGQAfv35696654 = -266557823;    int eqkYOGQAfv72737406 = -194191890;    int eqkYOGQAfv97551401 = -428707037;    int eqkYOGQAfv37859205 = -367175691;    int eqkYOGQAfv66335802 = -492282596;    int eqkYOGQAfv27010057 = -45364385;    int eqkYOGQAfv23706876 = -46839046;    int eqkYOGQAfv28912353 = -211748056;    int eqkYOGQAfv5908283 = -801308066;    int eqkYOGQAfv15702675 = -702052382;    int eqkYOGQAfv52439720 = -986049212;    int eqkYOGQAfv20458616 = -702741589;    int eqkYOGQAfv79321053 = -995349885;    int eqkYOGQAfv16092443 = -386596977;    int eqkYOGQAfv20672026 = -461125745;    int eqkYOGQAfv28016458 = -186388365;    int eqkYOGQAfv53449468 = 55845131;    int eqkYOGQAfv26934704 = -980666884;    int eqkYOGQAfv67688351 = -20794246;    int eqkYOGQAfv53613547 = -267189901;    int eqkYOGQAfv94360673 = 15119151;    int eqkYOGQAfv61370674 = -258888527;    int eqkYOGQAfv34191213 = -819692156;    int eqkYOGQAfv26289914 = -516579903;    int eqkYOGQAfv53936587 = -374999054;    int eqkYOGQAfv64483727 = -488778847;    int eqkYOGQAfv23918979 = -551959303;    int eqkYOGQAfv77361244 = -721561924;    int eqkYOGQAfv12462867 = 96399049;    int eqkYOGQAfv4401773 = 54523489;    int eqkYOGQAfv5466081 = -935296676;    int eqkYOGQAfv31975193 = -51235825;    int eqkYOGQAfv61357296 = -549482723;     eqkYOGQAfv48323231 = eqkYOGQAfv63788374;     eqkYOGQAfv63788374 = eqkYOGQAfv51295703;     eqkYOGQAfv51295703 = eqkYOGQAfv24561917;     eqkYOGQAfv24561917 = eqkYOGQAfv35909705;     eqkYOGQAfv35909705 = eqkYOGQAfv92282966;     eqkYOGQAfv92282966 = eqkYOGQAfv27756902;     eqkYOGQAfv27756902 = eqkYOGQAfv84992663;     eqkYOGQAfv84992663 = eqkYOGQAfv33790910;     eqkYOGQAfv33790910 = eqkYOGQAfv1734380;     eqkYOGQAfv1734380 = eqkYOGQAfv2610419;     eqkYOGQAfv2610419 = eqkYOGQAfv39080520;     eqkYOGQAfv39080520 = eqkYOGQAfv91288558;     eqkYOGQAfv91288558 = eqkYOGQAfv45299806;     eqkYOGQAfv45299806 = eqkYOGQAfv31732200;     eqkYOGQAfv31732200 = eqkYOGQAfv38032289;     eqkYOGQAfv38032289 = eqkYOGQAfv21760803;     eqkYOGQAfv21760803 = eqkYOGQAfv61814074;     eqkYOGQAfv61814074 = eqkYOGQAfv48741678;     eqkYOGQAfv48741678 = eqkYOGQAfv67162614;     eqkYOGQAfv67162614 = eqkYOGQAfv55673490;     eqkYOGQAfv55673490 = eqkYOGQAfv9952045;     eqkYOGQAfv9952045 = eqkYOGQAfv55897490;     eqkYOGQAfv55897490 = eqkYOGQAfv5059206;     eqkYOGQAfv5059206 = eqkYOGQAfv33620644;     eqkYOGQAfv33620644 = eqkYOGQAfv25024667;     eqkYOGQAfv25024667 = eqkYOGQAfv44609407;     eqkYOGQAfv44609407 = eqkYOGQAfv1969507;     eqkYOGQAfv1969507 = eqkYOGQAfv39629228;     eqkYOGQAfv39629228 = eqkYOGQAfv45007620;     eqkYOGQAfv45007620 = eqkYOGQAfv30251803;     eqkYOGQAfv30251803 = eqkYOGQAfv79367125;     eqkYOGQAfv79367125 = eqkYOGQAfv25737045;     eqkYOGQAfv25737045 = eqkYOGQAfv55591904;     eqkYOGQAfv55591904 = eqkYOGQAfv72562400;     eqkYOGQAfv72562400 = eqkYOGQAfv34180798;     eqkYOGQAfv34180798 = eqkYOGQAfv173085;     eqkYOGQAfv173085 = eqkYOGQAfv55425000;     eqkYOGQAfv55425000 = eqkYOGQAfv34804017;     eqkYOGQAfv34804017 = eqkYOGQAfv25034803;     eqkYOGQAfv25034803 = eqkYOGQAfv38250262;     eqkYOGQAfv38250262 = eqkYOGQAfv49765208;     eqkYOGQAfv49765208 = eqkYOGQAfv94249370;     eqkYOGQAfv94249370 = eqkYOGQAfv3457770;     eqkYOGQAfv3457770 = eqkYOGQAfv84600589;     eqkYOGQAfv84600589 = eqkYOGQAfv54299591;     eqkYOGQAfv54299591 = eqkYOGQAfv8932225;     eqkYOGQAfv8932225 = eqkYOGQAfv23937381;     eqkYOGQAfv23937381 = eqkYOGQAfv73953049;     eqkYOGQAfv73953049 = eqkYOGQAfv86179760;     eqkYOGQAfv86179760 = eqkYOGQAfv18072916;     eqkYOGQAfv18072916 = eqkYOGQAfv62563451;     eqkYOGQAfv62563451 = eqkYOGQAfv25753579;     eqkYOGQAfv25753579 = eqkYOGQAfv31376372;     eqkYOGQAfv31376372 = eqkYOGQAfv94221229;     eqkYOGQAfv94221229 = eqkYOGQAfv38371187;     eqkYOGQAfv38371187 = eqkYOGQAfv7890885;     eqkYOGQAfv7890885 = eqkYOGQAfv46236497;     eqkYOGQAfv46236497 = eqkYOGQAfv90941273;     eqkYOGQAfv90941273 = eqkYOGQAfv10885038;     eqkYOGQAfv10885038 = eqkYOGQAfv47673559;     eqkYOGQAfv47673559 = eqkYOGQAfv25787395;     eqkYOGQAfv25787395 = eqkYOGQAfv45363435;     eqkYOGQAfv45363435 = eqkYOGQAfv88783290;     eqkYOGQAfv88783290 = eqkYOGQAfv71482577;     eqkYOGQAfv71482577 = eqkYOGQAfv23243294;     eqkYOGQAfv23243294 = eqkYOGQAfv13343475;     eqkYOGQAfv13343475 = eqkYOGQAfv35696654;     eqkYOGQAfv35696654 = eqkYOGQAfv72737406;     eqkYOGQAfv72737406 = eqkYOGQAfv97551401;     eqkYOGQAfv97551401 = eqkYOGQAfv37859205;     eqkYOGQAfv37859205 = eqkYOGQAfv66335802;     eqkYOGQAfv66335802 = eqkYOGQAfv27010057;     eqkYOGQAfv27010057 = eqkYOGQAfv23706876;     eqkYOGQAfv23706876 = eqkYOGQAfv28912353;     eqkYOGQAfv28912353 = eqkYOGQAfv5908283;     eqkYOGQAfv5908283 = eqkYOGQAfv15702675;     eqkYOGQAfv15702675 = eqkYOGQAfv52439720;     eqkYOGQAfv52439720 = eqkYOGQAfv20458616;     eqkYOGQAfv20458616 = eqkYOGQAfv79321053;     eqkYOGQAfv79321053 = eqkYOGQAfv16092443;     eqkYOGQAfv16092443 = eqkYOGQAfv20672026;     eqkYOGQAfv20672026 = eqkYOGQAfv28016458;     eqkYOGQAfv28016458 = eqkYOGQAfv53449468;     eqkYOGQAfv53449468 = eqkYOGQAfv26934704;     eqkYOGQAfv26934704 = eqkYOGQAfv67688351;     eqkYOGQAfv67688351 = eqkYOGQAfv53613547;     eqkYOGQAfv53613547 = eqkYOGQAfv94360673;     eqkYOGQAfv94360673 = eqkYOGQAfv61370674;     eqkYOGQAfv61370674 = eqkYOGQAfv34191213;     eqkYOGQAfv34191213 = eqkYOGQAfv26289914;     eqkYOGQAfv26289914 = eqkYOGQAfv53936587;     eqkYOGQAfv53936587 = eqkYOGQAfv64483727;     eqkYOGQAfv64483727 = eqkYOGQAfv23918979;     eqkYOGQAfv23918979 = eqkYOGQAfv77361244;     eqkYOGQAfv77361244 = eqkYOGQAfv12462867;     eqkYOGQAfv12462867 = eqkYOGQAfv4401773;     eqkYOGQAfv4401773 = eqkYOGQAfv5466081;     eqkYOGQAfv5466081 = eqkYOGQAfv31975193;     eqkYOGQAfv31975193 = eqkYOGQAfv61357296;     eqkYOGQAfv61357296 = eqkYOGQAfv48323231;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void neYUFLvHij68108061() {     int MMdooMFAjJ31767537 = -330148404;    int MMdooMFAjJ80660463 = -193790915;    int MMdooMFAjJ47704210 = -230325500;    int MMdooMFAjJ87354168 = -353381714;    int MMdooMFAjJ42031029 = -275341374;    int MMdooMFAjJ90787366 = -209963404;    int MMdooMFAjJ98796219 = -348823978;    int MMdooMFAjJ99340106 = -964644293;    int MMdooMFAjJ92320610 = -214355140;    int MMdooMFAjJ34280032 = -496933688;    int MMdooMFAjJ52026357 = -940762234;    int MMdooMFAjJ2285919 = -377655443;    int MMdooMFAjJ9406335 = -508104546;    int MMdooMFAjJ24926698 = -660467938;    int MMdooMFAjJ92586663 = -51304429;    int MMdooMFAjJ24421451 = -491062461;    int MMdooMFAjJ15512688 = -777864714;    int MMdooMFAjJ42665520 = -236251235;    int MMdooMFAjJ11181821 = -318340423;    int MMdooMFAjJ56530057 = -387849516;    int MMdooMFAjJ11313877 = -923601819;    int MMdooMFAjJ22099363 = -782340958;    int MMdooMFAjJ59723198 = -222303418;    int MMdooMFAjJ39765764 = -647817278;    int MMdooMFAjJ75196602 = -775776393;    int MMdooMFAjJ20366816 = 42639072;    int MMdooMFAjJ94064147 = -607711511;    int MMdooMFAjJ41701132 = 64046969;    int MMdooMFAjJ75710322 = -352703194;    int MMdooMFAjJ60840961 = -231905511;    int MMdooMFAjJ48347116 = -621185593;    int MMdooMFAjJ42473553 = -802592546;    int MMdooMFAjJ69275932 = -395762379;    int MMdooMFAjJ97750414 = 58512347;    int MMdooMFAjJ51289131 = -775650427;    int MMdooMFAjJ46569983 = -316598316;    int MMdooMFAjJ68787263 = -114656624;    int MMdooMFAjJ96003832 = -140707612;    int MMdooMFAjJ83799095 = -28479771;    int MMdooMFAjJ74529938 = 74579970;    int MMdooMFAjJ71899186 = -218064441;    int MMdooMFAjJ252994 = 93326956;    int MMdooMFAjJ17816440 = -540194049;    int MMdooMFAjJ95362710 = 47452220;    int MMdooMFAjJ76545086 = -601407247;    int MMdooMFAjJ93905586 = -240384673;    int MMdooMFAjJ84947654 = -822558465;    int MMdooMFAjJ43607977 = -417223143;    int MMdooMFAjJ4800162 = -165211866;    int MMdooMFAjJ21512245 = -653063639;    int MMdooMFAjJ91330364 = 40709802;    int MMdooMFAjJ18064246 = -920893458;    int MMdooMFAjJ39866038 = -731954882;    int MMdooMFAjJ56053131 = -408422741;    int MMdooMFAjJ56794184 = -339483079;    int MMdooMFAjJ9668175 = -547807447;    int MMdooMFAjJ20937266 = -971487498;    int MMdooMFAjJ7938446 = -582508223;    int MMdooMFAjJ12157567 = -577605322;    int MMdooMFAjJ21664213 = -217980446;    int MMdooMFAjJ96723219 = -602251894;    int MMdooMFAjJ57095087 = -312870948;    int MMdooMFAjJ23629784 = -511941100;    int MMdooMFAjJ31479650 = -982449629;    int MMdooMFAjJ85932915 = -875748096;    int MMdooMFAjJ9552804 = -38169688;    int MMdooMFAjJ33009987 = -981893065;    int MMdooMFAjJ11655920 = -466616894;    int MMdooMFAjJ73637567 = -884817511;    int MMdooMFAjJ46016681 = -734706113;    int MMdooMFAjJ55634187 = -276405837;    int MMdooMFAjJ19508856 = -537157103;    int MMdooMFAjJ58866425 = -107771464;    int MMdooMFAjJ36651883 = -292920394;    int MMdooMFAjJ84630870 = -69785075;    int MMdooMFAjJ11060884 = -916928776;    int MMdooMFAjJ4282923 = -142146910;    int MMdooMFAjJ64360488 = -169755638;    int MMdooMFAjJ63220677 = 53589968;    int MMdooMFAjJ81291015 = -435391720;    int MMdooMFAjJ35419161 = -134802463;    int MMdooMFAjJ50456171 = -90488369;    int MMdooMFAjJ36900971 = -770741166;    int MMdooMFAjJ54198078 = -699639556;    int MMdooMFAjJ69510597 = -172615313;    int MMdooMFAjJ30282871 = -700292135;    int MMdooMFAjJ2607515 = 29362336;    int MMdooMFAjJ13222802 = -987339639;    int MMdooMFAjJ40956230 = -602004575;    int MMdooMFAjJ41620956 = -127842981;    int MMdooMFAjJ25632717 = -345110819;    int MMdooMFAjJ60848817 = -532148402;    int MMdooMFAjJ83846265 = -563102290;    int MMdooMFAjJ62134882 = -810499325;    int MMdooMFAjJ77806718 = -323168136;    int MMdooMFAjJ14804099 = -905193494;    int MMdooMFAjJ76623210 = -394731945;    int MMdooMFAjJ86336790 = -557744420;    int MMdooMFAjJ9429795 = -76799685;    int MMdooMFAjJ66992282 = -330148404;     MMdooMFAjJ31767537 = MMdooMFAjJ80660463;     MMdooMFAjJ80660463 = MMdooMFAjJ47704210;     MMdooMFAjJ47704210 = MMdooMFAjJ87354168;     MMdooMFAjJ87354168 = MMdooMFAjJ42031029;     MMdooMFAjJ42031029 = MMdooMFAjJ90787366;     MMdooMFAjJ90787366 = MMdooMFAjJ98796219;     MMdooMFAjJ98796219 = MMdooMFAjJ99340106;     MMdooMFAjJ99340106 = MMdooMFAjJ92320610;     MMdooMFAjJ92320610 = MMdooMFAjJ34280032;     MMdooMFAjJ34280032 = MMdooMFAjJ52026357;     MMdooMFAjJ52026357 = MMdooMFAjJ2285919;     MMdooMFAjJ2285919 = MMdooMFAjJ9406335;     MMdooMFAjJ9406335 = MMdooMFAjJ24926698;     MMdooMFAjJ24926698 = MMdooMFAjJ92586663;     MMdooMFAjJ92586663 = MMdooMFAjJ24421451;     MMdooMFAjJ24421451 = MMdooMFAjJ15512688;     MMdooMFAjJ15512688 = MMdooMFAjJ42665520;     MMdooMFAjJ42665520 = MMdooMFAjJ11181821;     MMdooMFAjJ11181821 = MMdooMFAjJ56530057;     MMdooMFAjJ56530057 = MMdooMFAjJ11313877;     MMdooMFAjJ11313877 = MMdooMFAjJ22099363;     MMdooMFAjJ22099363 = MMdooMFAjJ59723198;     MMdooMFAjJ59723198 = MMdooMFAjJ39765764;     MMdooMFAjJ39765764 = MMdooMFAjJ75196602;     MMdooMFAjJ75196602 = MMdooMFAjJ20366816;     MMdooMFAjJ20366816 = MMdooMFAjJ94064147;     MMdooMFAjJ94064147 = MMdooMFAjJ41701132;     MMdooMFAjJ41701132 = MMdooMFAjJ75710322;     MMdooMFAjJ75710322 = MMdooMFAjJ60840961;     MMdooMFAjJ60840961 = MMdooMFAjJ48347116;     MMdooMFAjJ48347116 = MMdooMFAjJ42473553;     MMdooMFAjJ42473553 = MMdooMFAjJ69275932;     MMdooMFAjJ69275932 = MMdooMFAjJ97750414;     MMdooMFAjJ97750414 = MMdooMFAjJ51289131;     MMdooMFAjJ51289131 = MMdooMFAjJ46569983;     MMdooMFAjJ46569983 = MMdooMFAjJ68787263;     MMdooMFAjJ68787263 = MMdooMFAjJ96003832;     MMdooMFAjJ96003832 = MMdooMFAjJ83799095;     MMdooMFAjJ83799095 = MMdooMFAjJ74529938;     MMdooMFAjJ74529938 = MMdooMFAjJ71899186;     MMdooMFAjJ71899186 = MMdooMFAjJ252994;     MMdooMFAjJ252994 = MMdooMFAjJ17816440;     MMdooMFAjJ17816440 = MMdooMFAjJ95362710;     MMdooMFAjJ95362710 = MMdooMFAjJ76545086;     MMdooMFAjJ76545086 = MMdooMFAjJ93905586;     MMdooMFAjJ93905586 = MMdooMFAjJ84947654;     MMdooMFAjJ84947654 = MMdooMFAjJ43607977;     MMdooMFAjJ43607977 = MMdooMFAjJ4800162;     MMdooMFAjJ4800162 = MMdooMFAjJ21512245;     MMdooMFAjJ21512245 = MMdooMFAjJ91330364;     MMdooMFAjJ91330364 = MMdooMFAjJ18064246;     MMdooMFAjJ18064246 = MMdooMFAjJ39866038;     MMdooMFAjJ39866038 = MMdooMFAjJ56053131;     MMdooMFAjJ56053131 = MMdooMFAjJ56794184;     MMdooMFAjJ56794184 = MMdooMFAjJ9668175;     MMdooMFAjJ9668175 = MMdooMFAjJ20937266;     MMdooMFAjJ20937266 = MMdooMFAjJ7938446;     MMdooMFAjJ7938446 = MMdooMFAjJ12157567;     MMdooMFAjJ12157567 = MMdooMFAjJ21664213;     MMdooMFAjJ21664213 = MMdooMFAjJ96723219;     MMdooMFAjJ96723219 = MMdooMFAjJ57095087;     MMdooMFAjJ57095087 = MMdooMFAjJ23629784;     MMdooMFAjJ23629784 = MMdooMFAjJ31479650;     MMdooMFAjJ31479650 = MMdooMFAjJ85932915;     MMdooMFAjJ85932915 = MMdooMFAjJ9552804;     MMdooMFAjJ9552804 = MMdooMFAjJ33009987;     MMdooMFAjJ33009987 = MMdooMFAjJ11655920;     MMdooMFAjJ11655920 = MMdooMFAjJ73637567;     MMdooMFAjJ73637567 = MMdooMFAjJ46016681;     MMdooMFAjJ46016681 = MMdooMFAjJ55634187;     MMdooMFAjJ55634187 = MMdooMFAjJ19508856;     MMdooMFAjJ19508856 = MMdooMFAjJ58866425;     MMdooMFAjJ58866425 = MMdooMFAjJ36651883;     MMdooMFAjJ36651883 = MMdooMFAjJ84630870;     MMdooMFAjJ84630870 = MMdooMFAjJ11060884;     MMdooMFAjJ11060884 = MMdooMFAjJ4282923;     MMdooMFAjJ4282923 = MMdooMFAjJ64360488;     MMdooMFAjJ64360488 = MMdooMFAjJ63220677;     MMdooMFAjJ63220677 = MMdooMFAjJ81291015;     MMdooMFAjJ81291015 = MMdooMFAjJ35419161;     MMdooMFAjJ35419161 = MMdooMFAjJ50456171;     MMdooMFAjJ50456171 = MMdooMFAjJ36900971;     MMdooMFAjJ36900971 = MMdooMFAjJ54198078;     MMdooMFAjJ54198078 = MMdooMFAjJ69510597;     MMdooMFAjJ69510597 = MMdooMFAjJ30282871;     MMdooMFAjJ30282871 = MMdooMFAjJ2607515;     MMdooMFAjJ2607515 = MMdooMFAjJ13222802;     MMdooMFAjJ13222802 = MMdooMFAjJ40956230;     MMdooMFAjJ40956230 = MMdooMFAjJ41620956;     MMdooMFAjJ41620956 = MMdooMFAjJ25632717;     MMdooMFAjJ25632717 = MMdooMFAjJ60848817;     MMdooMFAjJ60848817 = MMdooMFAjJ83846265;     MMdooMFAjJ83846265 = MMdooMFAjJ62134882;     MMdooMFAjJ62134882 = MMdooMFAjJ77806718;     MMdooMFAjJ77806718 = MMdooMFAjJ14804099;     MMdooMFAjJ14804099 = MMdooMFAjJ76623210;     MMdooMFAjJ76623210 = MMdooMFAjJ86336790;     MMdooMFAjJ86336790 = MMdooMFAjJ9429795;     MMdooMFAjJ9429795 = MMdooMFAjJ66992282;     MMdooMFAjJ66992282 = MMdooMFAjJ31767537;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XAjPXDyQoF38563934() {     int CbsexJjkNd44553704 = -465189096;    int CbsexJjkNd67431352 = -364124928;    int CbsexJjkNd65457009 = -638684882;    int CbsexJjkNd95294994 = -177583991;    int CbsexJjkNd35542133 = -767157839;    int CbsexJjkNd34097438 = 13113282;    int CbsexJjkNd20003978 = -894496049;    int CbsexJjkNd26059219 = -309162550;    int CbsexJjkNd87725421 = -176113232;    int CbsexJjkNd40498736 = -301021321;    int CbsexJjkNd79513739 = 44551751;    int CbsexJjkNd34236424 = -884988259;    int CbsexJjkNd79549637 = -411887203;    int CbsexJjkNd74550776 = -992883136;    int CbsexJjkNd15893217 = 41709741;    int CbsexJjkNd52846519 = 58046150;    int CbsexJjkNd71264505 = -441639181;    int CbsexJjkNd26224006 = -891461537;    int CbsexJjkNd12342451 = -227809619;    int CbsexJjkNd59184553 = -987093655;    int CbsexJjkNd91487933 = -314918083;    int CbsexJjkNd51147182 = -952199306;    int CbsexJjkNd98483493 = -153307312;    int CbsexJjkNd84272893 = 91837709;    int CbsexJjkNd81090861 = -986218317;    int CbsexJjkNd1685461 = -747792127;    int CbsexJjkNd5479684 = -141417791;    int CbsexJjkNd9054057 = -109751136;    int CbsexJjkNd92327375 = -566920435;    int CbsexJjkNd13592077 = -961791666;    int CbsexJjkNd92783112 = -897602343;    int CbsexJjkNd45792376 = -781312224;    int CbsexJjkNd15321881 = -271617164;    int CbsexJjkNd74826787 = -946241871;    int CbsexJjkNd34349663 = -285682308;    int CbsexJjkNd41794097 = -105161420;    int CbsexJjkNd5551324 = 95434025;    int CbsexJjkNd53707855 = -60327703;    int CbsexJjkNd89574481 = -970330348;    int CbsexJjkNd794668 = -869836875;    int CbsexJjkNd33330495 = -593538435;    int CbsexJjkNd7955450 = -904984378;    int CbsexJjkNd52699910 = -563627494;    int CbsexJjkNd39991248 = -293720023;    int CbsexJjkNd56553576 = -494177207;    int CbsexJjkNd67533716 = -313703113;    int CbsexJjkNd62529674 = -220073518;    int CbsexJjkNd73047943 = -572575922;    int CbsexJjkNd48864211 = -328813840;    int CbsexJjkNd68462665 = -164100102;    int CbsexJjkNd9657858 = -212174507;    int CbsexJjkNd46240793 = -292078796;    int CbsexJjkNd74351188 = -907958165;    int CbsexJjkNd752857 = 2125517;    int CbsexJjkNd47076001 = -333927565;    int CbsexJjkNd93406522 = -512989790;    int CbsexJjkNd68947858 = -110817616;    int CbsexJjkNd81184116 = -630522592;    int CbsexJjkNd14204133 = -191365674;    int CbsexJjkNd33856672 = 80634288;    int CbsexJjkNd28617754 = -845468928;    int CbsexJjkNd10949921 = -684744913;    int CbsexJjkNd33731844 = -742242116;    int CbsexJjkNd74133344 = -214321567;    int CbsexJjkNd47715624 = -403418978;    int CbsexJjkNd33721363 = -174136025;    int CbsexJjkNd18914543 = -513371096;    int CbsexJjkNd4722850 = -465645333;    int CbsexJjkNd40201114 = -607200829;    int CbsexJjkNd74099119 = -853128840;    int CbsexJjkNd47295196 = 62612124;    int CbsexJjkNd17556651 = -281311478;    int CbsexJjkNd36649524 = -921131189;    int CbsexJjkNd11547784 = -357972744;    int CbsexJjkNd25854059 = -293555221;    int CbsexJjkNd83532484 = -409933705;    int CbsexJjkNd98447272 = -288571813;    int CbsexJjkNd58492245 = -859587290;    int CbsexJjkNd27719317 = -413985084;    int CbsexJjkNd13557146 = -572515205;    int CbsexJjkNd39155787 = -427718610;    int CbsexJjkNd32431741 = -568841869;    int CbsexJjkNd60189846 = -780937296;    int CbsexJjkNd23864711 = -302820333;    int CbsexJjkNd3934219 = -649617159;    int CbsexJjkNd46542320 = -505523547;    int CbsexJjkNd71441188 = -873354060;    int CbsexJjkNd14569025 = -173742682;    int CbsexJjkNd27750787 = -512314306;    int CbsexJjkNd40943141 = -772692518;    int CbsexJjkNd72846239 = -994343804;    int CbsexJjkNd24367208 = -274043383;    int CbsexJjkNd39503722 = -868962029;    int CbsexJjkNd55717809 = -950964636;    int CbsexJjkNd72176913 = 75632052;    int CbsexJjkNd22380574 = -908793522;    int CbsexJjkNd74223606 = -62742263;    int CbsexJjkNd78566565 = -249305927;    int CbsexJjkNd92275624 = -890301045;    int CbsexJjkNd22832214 = -465189096;     CbsexJjkNd44553704 = CbsexJjkNd67431352;     CbsexJjkNd67431352 = CbsexJjkNd65457009;     CbsexJjkNd65457009 = CbsexJjkNd95294994;     CbsexJjkNd95294994 = CbsexJjkNd35542133;     CbsexJjkNd35542133 = CbsexJjkNd34097438;     CbsexJjkNd34097438 = CbsexJjkNd20003978;     CbsexJjkNd20003978 = CbsexJjkNd26059219;     CbsexJjkNd26059219 = CbsexJjkNd87725421;     CbsexJjkNd87725421 = CbsexJjkNd40498736;     CbsexJjkNd40498736 = CbsexJjkNd79513739;     CbsexJjkNd79513739 = CbsexJjkNd34236424;     CbsexJjkNd34236424 = CbsexJjkNd79549637;     CbsexJjkNd79549637 = CbsexJjkNd74550776;     CbsexJjkNd74550776 = CbsexJjkNd15893217;     CbsexJjkNd15893217 = CbsexJjkNd52846519;     CbsexJjkNd52846519 = CbsexJjkNd71264505;     CbsexJjkNd71264505 = CbsexJjkNd26224006;     CbsexJjkNd26224006 = CbsexJjkNd12342451;     CbsexJjkNd12342451 = CbsexJjkNd59184553;     CbsexJjkNd59184553 = CbsexJjkNd91487933;     CbsexJjkNd91487933 = CbsexJjkNd51147182;     CbsexJjkNd51147182 = CbsexJjkNd98483493;     CbsexJjkNd98483493 = CbsexJjkNd84272893;     CbsexJjkNd84272893 = CbsexJjkNd81090861;     CbsexJjkNd81090861 = CbsexJjkNd1685461;     CbsexJjkNd1685461 = CbsexJjkNd5479684;     CbsexJjkNd5479684 = CbsexJjkNd9054057;     CbsexJjkNd9054057 = CbsexJjkNd92327375;     CbsexJjkNd92327375 = CbsexJjkNd13592077;     CbsexJjkNd13592077 = CbsexJjkNd92783112;     CbsexJjkNd92783112 = CbsexJjkNd45792376;     CbsexJjkNd45792376 = CbsexJjkNd15321881;     CbsexJjkNd15321881 = CbsexJjkNd74826787;     CbsexJjkNd74826787 = CbsexJjkNd34349663;     CbsexJjkNd34349663 = CbsexJjkNd41794097;     CbsexJjkNd41794097 = CbsexJjkNd5551324;     CbsexJjkNd5551324 = CbsexJjkNd53707855;     CbsexJjkNd53707855 = CbsexJjkNd89574481;     CbsexJjkNd89574481 = CbsexJjkNd794668;     CbsexJjkNd794668 = CbsexJjkNd33330495;     CbsexJjkNd33330495 = CbsexJjkNd7955450;     CbsexJjkNd7955450 = CbsexJjkNd52699910;     CbsexJjkNd52699910 = CbsexJjkNd39991248;     CbsexJjkNd39991248 = CbsexJjkNd56553576;     CbsexJjkNd56553576 = CbsexJjkNd67533716;     CbsexJjkNd67533716 = CbsexJjkNd62529674;     CbsexJjkNd62529674 = CbsexJjkNd73047943;     CbsexJjkNd73047943 = CbsexJjkNd48864211;     CbsexJjkNd48864211 = CbsexJjkNd68462665;     CbsexJjkNd68462665 = CbsexJjkNd9657858;     CbsexJjkNd9657858 = CbsexJjkNd46240793;     CbsexJjkNd46240793 = CbsexJjkNd74351188;     CbsexJjkNd74351188 = CbsexJjkNd752857;     CbsexJjkNd752857 = CbsexJjkNd47076001;     CbsexJjkNd47076001 = CbsexJjkNd93406522;     CbsexJjkNd93406522 = CbsexJjkNd68947858;     CbsexJjkNd68947858 = CbsexJjkNd81184116;     CbsexJjkNd81184116 = CbsexJjkNd14204133;     CbsexJjkNd14204133 = CbsexJjkNd33856672;     CbsexJjkNd33856672 = CbsexJjkNd28617754;     CbsexJjkNd28617754 = CbsexJjkNd10949921;     CbsexJjkNd10949921 = CbsexJjkNd33731844;     CbsexJjkNd33731844 = CbsexJjkNd74133344;     CbsexJjkNd74133344 = CbsexJjkNd47715624;     CbsexJjkNd47715624 = CbsexJjkNd33721363;     CbsexJjkNd33721363 = CbsexJjkNd18914543;     CbsexJjkNd18914543 = CbsexJjkNd4722850;     CbsexJjkNd4722850 = CbsexJjkNd40201114;     CbsexJjkNd40201114 = CbsexJjkNd74099119;     CbsexJjkNd74099119 = CbsexJjkNd47295196;     CbsexJjkNd47295196 = CbsexJjkNd17556651;     CbsexJjkNd17556651 = CbsexJjkNd36649524;     CbsexJjkNd36649524 = CbsexJjkNd11547784;     CbsexJjkNd11547784 = CbsexJjkNd25854059;     CbsexJjkNd25854059 = CbsexJjkNd83532484;     CbsexJjkNd83532484 = CbsexJjkNd98447272;     CbsexJjkNd98447272 = CbsexJjkNd58492245;     CbsexJjkNd58492245 = CbsexJjkNd27719317;     CbsexJjkNd27719317 = CbsexJjkNd13557146;     CbsexJjkNd13557146 = CbsexJjkNd39155787;     CbsexJjkNd39155787 = CbsexJjkNd32431741;     CbsexJjkNd32431741 = CbsexJjkNd60189846;     CbsexJjkNd60189846 = CbsexJjkNd23864711;     CbsexJjkNd23864711 = CbsexJjkNd3934219;     CbsexJjkNd3934219 = CbsexJjkNd46542320;     CbsexJjkNd46542320 = CbsexJjkNd71441188;     CbsexJjkNd71441188 = CbsexJjkNd14569025;     CbsexJjkNd14569025 = CbsexJjkNd27750787;     CbsexJjkNd27750787 = CbsexJjkNd40943141;     CbsexJjkNd40943141 = CbsexJjkNd72846239;     CbsexJjkNd72846239 = CbsexJjkNd24367208;     CbsexJjkNd24367208 = CbsexJjkNd39503722;     CbsexJjkNd39503722 = CbsexJjkNd55717809;     CbsexJjkNd55717809 = CbsexJjkNd72176913;     CbsexJjkNd72176913 = CbsexJjkNd22380574;     CbsexJjkNd22380574 = CbsexJjkNd74223606;     CbsexJjkNd74223606 = CbsexJjkNd78566565;     CbsexJjkNd78566565 = CbsexJjkNd92275624;     CbsexJjkNd92275624 = CbsexJjkNd22832214;     CbsexJjkNd22832214 = CbsexJjkNd44553704;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XFzfoVHJhF8689610() {     int tqjlmZPrfW12000741 = -65233230;    int tqjlmZPrfW62627543 = 28571369;    int tqjlmZPrfW42629244 = -375300264;    int tqjlmZPrfW89590900 = -187347849;    int tqjlmZPrfW94664806 = -463054222;    int tqjlmZPrfW56776473 = -193528056;    int tqjlmZPrfW93765103 = -937524325;    int tqjlmZPrfW12739385 = -450071722;    int tqjlmZPrfW88164478 = -471078625;    int tqjlmZPrfW32709913 = -747103352;    int tqjlmZPrfW65291811 = -736018765;    int tqjlmZPrfW14659965 = -743266801;    int tqjlmZPrfW81657340 = -97551690;    int tqjlmZPrfW63297066 = -592248861;    int tqjlmZPrfW24251734 = -909684870;    int tqjlmZPrfW96474576 = -890209153;    int tqjlmZPrfW70995327 = -759398907;    int tqjlmZPrfW54378545 = -113164684;    int tqjlmZPrfW94295412 = -905108095;    int tqjlmZPrfW7399843 = -219679489;    int tqjlmZPrfW26166954 = -563851125;    int tqjlmZPrfW53151324 = -778509878;    int tqjlmZPrfW36835224 = -212096786;    int tqjlmZPrfW86558283 = -86627597;    int tqjlmZPrfW54295047 = -42879841;    int tqjlmZPrfW7670977 = -533726183;    int tqjlmZPrfW48663562 = 70831363;    int tqjlmZPrfW94252457 = -782634113;    int tqjlmZPrfW36163659 = -515312235;    int tqjlmZPrfW84688731 = -107004415;    int tqjlmZPrfW55511958 = -235798384;    int tqjlmZPrfW95378952 = -779706670;    int tqjlmZPrfW43571284 = -502045346;    int tqjlmZPrfW46276216 = -654435166;    int tqjlmZPrfW69556313 = -854263523;    int tqjlmZPrfW81516564 = -61954375;    int tqjlmZPrfW5078505 = -545248567;    int tqjlmZPrfW54471437 = -958614137;    int tqjlmZPrfW10805920 = -481448111;    int tqjlmZPrfW79172899 = -763130634;    int tqjlmZPrfW62865536 = -993618688;    int tqjlmZPrfW66634903 = -117035717;    int tqjlmZPrfW18300203 = -968436002;    int tqjlmZPrfW34382287 = -551607488;    int tqjlmZPrfW12528078 = -46205364;    int tqjlmZPrfW56067479 = -643766496;    int tqjlmZPrfW28887613 = -263365332;    int tqjlmZPrfW50531638 = -96819718;    int tqjlmZPrfW16612638 = -828382143;    int tqjlmZPrfW40209325 = -328130973;    int tqjlmZPrfW89839521 = -543719310;    int tqjlmZPrfW49252232 = -896184369;    int tqjlmZPrfW65115993 = -931822897;    int tqjlmZPrfW92618067 = -51292858;    int tqjlmZPrfW82522655 = -967381455;    int tqjlmZPrfW58849416 = -286723352;    int tqjlmZPrfW25792319 = -759331845;    int tqjlmZPrfW56070960 = -188672667;    int tqjlmZPrfW35295854 = -44468009;    int tqjlmZPrfW86993829 = -929328040;    int tqjlmZPrfW8112911 = -164359419;    int tqjlmZPrfW99512646 = -54890213;    int tqjlmZPrfW76575726 = -934759488;    int tqjlmZPrfW3475747 = -264074211;    int tqjlmZPrfW77197954 = -411304968;    int tqjlmZPrfW69912858 = -956312095;    int tqjlmZPrfW71088681 = -141221456;    int tqjlmZPrfW35381125 = -443116524;    int tqjlmZPrfW93740752 = -737985339;    int tqjlmZPrfW42735169 = -747730496;    int tqjlmZPrfW91396072 = -244960586;    int tqjlmZPrfW16523890 = -800784770;    int tqjlmZPrfW43572625 = -631716574;    int tqjlmZPrfW15122513 = -41977461;    int tqjlmZPrfW44534306 = -226060802;    int tqjlmZPrfW59532051 = -346815408;    int tqjlmZPrfW34851122 = -810073877;    int tqjlmZPrfW2452938 = -660489298;    int tqjlmZPrfW74030205 = 59577767;    int tqjlmZPrfW98227568 = -399113345;    int tqjlmZPrfW78783363 = -170360852;    int tqjlmZPrfW98131924 = -832348920;    int tqjlmZPrfW77639819 = -954251971;    int tqjlmZPrfW95954333 = -87181263;    int tqjlmZPrfW94849210 = -563285105;    int tqjlmZPrfW6259727 = -339614016;    int tqjlmZPrfW30262960 = -847883773;    int tqjlmZPrfW50953216 = -350752489;    int tqjlmZPrfW63753560 = -687053712;    int tqjlmZPrfW10706898 = -467540171;    int tqjlmZPrfW55724246 = -302622530;    int tqjlmZPrfW49007544 = -256575901;    int tqjlmZPrfW19175584 = -814146128;    int tqjlmZPrfW23812090 = -552120072;    int tqjlmZPrfW71059988 = -498771216;    int tqjlmZPrfW63352889 = -838728476;    int tqjlmZPrfW90059177 = -182276230;    int tqjlmZPrfW14824456 = -604361792;    int tqjlmZPrfW57184332 = -40302520;    int tqjlmZPrfW42615220 = -65233230;     tqjlmZPrfW12000741 = tqjlmZPrfW62627543;     tqjlmZPrfW62627543 = tqjlmZPrfW42629244;     tqjlmZPrfW42629244 = tqjlmZPrfW89590900;     tqjlmZPrfW89590900 = tqjlmZPrfW94664806;     tqjlmZPrfW94664806 = tqjlmZPrfW56776473;     tqjlmZPrfW56776473 = tqjlmZPrfW93765103;     tqjlmZPrfW93765103 = tqjlmZPrfW12739385;     tqjlmZPrfW12739385 = tqjlmZPrfW88164478;     tqjlmZPrfW88164478 = tqjlmZPrfW32709913;     tqjlmZPrfW32709913 = tqjlmZPrfW65291811;     tqjlmZPrfW65291811 = tqjlmZPrfW14659965;     tqjlmZPrfW14659965 = tqjlmZPrfW81657340;     tqjlmZPrfW81657340 = tqjlmZPrfW63297066;     tqjlmZPrfW63297066 = tqjlmZPrfW24251734;     tqjlmZPrfW24251734 = tqjlmZPrfW96474576;     tqjlmZPrfW96474576 = tqjlmZPrfW70995327;     tqjlmZPrfW70995327 = tqjlmZPrfW54378545;     tqjlmZPrfW54378545 = tqjlmZPrfW94295412;     tqjlmZPrfW94295412 = tqjlmZPrfW7399843;     tqjlmZPrfW7399843 = tqjlmZPrfW26166954;     tqjlmZPrfW26166954 = tqjlmZPrfW53151324;     tqjlmZPrfW53151324 = tqjlmZPrfW36835224;     tqjlmZPrfW36835224 = tqjlmZPrfW86558283;     tqjlmZPrfW86558283 = tqjlmZPrfW54295047;     tqjlmZPrfW54295047 = tqjlmZPrfW7670977;     tqjlmZPrfW7670977 = tqjlmZPrfW48663562;     tqjlmZPrfW48663562 = tqjlmZPrfW94252457;     tqjlmZPrfW94252457 = tqjlmZPrfW36163659;     tqjlmZPrfW36163659 = tqjlmZPrfW84688731;     tqjlmZPrfW84688731 = tqjlmZPrfW55511958;     tqjlmZPrfW55511958 = tqjlmZPrfW95378952;     tqjlmZPrfW95378952 = tqjlmZPrfW43571284;     tqjlmZPrfW43571284 = tqjlmZPrfW46276216;     tqjlmZPrfW46276216 = tqjlmZPrfW69556313;     tqjlmZPrfW69556313 = tqjlmZPrfW81516564;     tqjlmZPrfW81516564 = tqjlmZPrfW5078505;     tqjlmZPrfW5078505 = tqjlmZPrfW54471437;     tqjlmZPrfW54471437 = tqjlmZPrfW10805920;     tqjlmZPrfW10805920 = tqjlmZPrfW79172899;     tqjlmZPrfW79172899 = tqjlmZPrfW62865536;     tqjlmZPrfW62865536 = tqjlmZPrfW66634903;     tqjlmZPrfW66634903 = tqjlmZPrfW18300203;     tqjlmZPrfW18300203 = tqjlmZPrfW34382287;     tqjlmZPrfW34382287 = tqjlmZPrfW12528078;     tqjlmZPrfW12528078 = tqjlmZPrfW56067479;     tqjlmZPrfW56067479 = tqjlmZPrfW28887613;     tqjlmZPrfW28887613 = tqjlmZPrfW50531638;     tqjlmZPrfW50531638 = tqjlmZPrfW16612638;     tqjlmZPrfW16612638 = tqjlmZPrfW40209325;     tqjlmZPrfW40209325 = tqjlmZPrfW89839521;     tqjlmZPrfW89839521 = tqjlmZPrfW49252232;     tqjlmZPrfW49252232 = tqjlmZPrfW65115993;     tqjlmZPrfW65115993 = tqjlmZPrfW92618067;     tqjlmZPrfW92618067 = tqjlmZPrfW82522655;     tqjlmZPrfW82522655 = tqjlmZPrfW58849416;     tqjlmZPrfW58849416 = tqjlmZPrfW25792319;     tqjlmZPrfW25792319 = tqjlmZPrfW56070960;     tqjlmZPrfW56070960 = tqjlmZPrfW35295854;     tqjlmZPrfW35295854 = tqjlmZPrfW86993829;     tqjlmZPrfW86993829 = tqjlmZPrfW8112911;     tqjlmZPrfW8112911 = tqjlmZPrfW99512646;     tqjlmZPrfW99512646 = tqjlmZPrfW76575726;     tqjlmZPrfW76575726 = tqjlmZPrfW3475747;     tqjlmZPrfW3475747 = tqjlmZPrfW77197954;     tqjlmZPrfW77197954 = tqjlmZPrfW69912858;     tqjlmZPrfW69912858 = tqjlmZPrfW71088681;     tqjlmZPrfW71088681 = tqjlmZPrfW35381125;     tqjlmZPrfW35381125 = tqjlmZPrfW93740752;     tqjlmZPrfW93740752 = tqjlmZPrfW42735169;     tqjlmZPrfW42735169 = tqjlmZPrfW91396072;     tqjlmZPrfW91396072 = tqjlmZPrfW16523890;     tqjlmZPrfW16523890 = tqjlmZPrfW43572625;     tqjlmZPrfW43572625 = tqjlmZPrfW15122513;     tqjlmZPrfW15122513 = tqjlmZPrfW44534306;     tqjlmZPrfW44534306 = tqjlmZPrfW59532051;     tqjlmZPrfW59532051 = tqjlmZPrfW34851122;     tqjlmZPrfW34851122 = tqjlmZPrfW2452938;     tqjlmZPrfW2452938 = tqjlmZPrfW74030205;     tqjlmZPrfW74030205 = tqjlmZPrfW98227568;     tqjlmZPrfW98227568 = tqjlmZPrfW78783363;     tqjlmZPrfW78783363 = tqjlmZPrfW98131924;     tqjlmZPrfW98131924 = tqjlmZPrfW77639819;     tqjlmZPrfW77639819 = tqjlmZPrfW95954333;     tqjlmZPrfW95954333 = tqjlmZPrfW94849210;     tqjlmZPrfW94849210 = tqjlmZPrfW6259727;     tqjlmZPrfW6259727 = tqjlmZPrfW30262960;     tqjlmZPrfW30262960 = tqjlmZPrfW50953216;     tqjlmZPrfW50953216 = tqjlmZPrfW63753560;     tqjlmZPrfW63753560 = tqjlmZPrfW10706898;     tqjlmZPrfW10706898 = tqjlmZPrfW55724246;     tqjlmZPrfW55724246 = tqjlmZPrfW49007544;     tqjlmZPrfW49007544 = tqjlmZPrfW19175584;     tqjlmZPrfW19175584 = tqjlmZPrfW23812090;     tqjlmZPrfW23812090 = tqjlmZPrfW71059988;     tqjlmZPrfW71059988 = tqjlmZPrfW63352889;     tqjlmZPrfW63352889 = tqjlmZPrfW90059177;     tqjlmZPrfW90059177 = tqjlmZPrfW14824456;     tqjlmZPrfW14824456 = tqjlmZPrfW57184332;     tqjlmZPrfW57184332 = tqjlmZPrfW42615220;     tqjlmZPrfW42615220 = tqjlmZPrfW12000741;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kohSKCAMiT26902952() {     int BEsWlsGYrh95445045 = -945898911;    int BEsWlsGYrh79499632 = 29189182;    int BEsWlsGYrh39037751 = -168825491;    int BEsWlsGYrh52383152 = -381147685;    int BEsWlsGYrh786130 = -269921713;    int BEsWlsGYrh55280874 = 27400291;    int BEsWlsGYrh64804421 = -677435638;    int BEsWlsGYrh27086827 = -437229752;    int BEsWlsGYrh46694179 = -812537976;    int BEsWlsGYrh65255565 = -115479464;    int BEsWlsGYrh14707749 = -926134637;    int BEsWlsGYrh77865364 = -730885048;    int BEsWlsGYrh99775116 = -989212931;    int BEsWlsGYrh42923958 = -380539218;    int BEsWlsGYrh85106197 = 96292148;    int BEsWlsGYrh82863738 = -678288478;    int BEsWlsGYrh64747212 = -512743934;    int BEsWlsGYrh35229991 = -360469561;    int BEsWlsGYrh56735554 = -903782966;    int BEsWlsGYrh96767284 = -749265481;    int BEsWlsGYrh81807341 = -153380156;    int BEsWlsGYrh65298642 = -769661649;    int BEsWlsGYrh40660932 = -251985983;    int BEsWlsGYrh21264842 = -158452993;    int BEsWlsGYrh95871004 = -602532600;    int BEsWlsGYrh3013126 = -414235898;    int BEsWlsGYrh98118302 = -657252980;    int BEsWlsGYrh33984083 = -852588997;    int BEsWlsGYrh72244752 = -377817377;    int BEsWlsGYrh522073 = -619854266;    int BEsWlsGYrh73607272 = -973555585;    int BEsWlsGYrh58485380 = -694901749;    int BEsWlsGYrh87110170 = -122917524;    int BEsWlsGYrh88434726 = -246037336;    int BEsWlsGYrh48283044 = 48071742;    int BEsWlsGYrh93905748 = -949978281;    int BEsWlsGYrh73692682 = -424097747;    int BEsWlsGYrh95050269 = -667084657;    int BEsWlsGYrh59800998 = -666345908;    int BEsWlsGYrh28668035 = -515724157;    int BEsWlsGYrh96514460 = -565167661;    int BEsWlsGYrh17122689 = -690944039;    int BEsWlsGYrh41867272 = -419493245;    int BEsWlsGYrh26287227 = -307790259;    int BEsWlsGYrh4472575 = -874362317;    int BEsWlsGYrh95673474 = -663377420;    int BEsWlsGYrh4903044 = -395669561;    int BEsWlsGYrh70202233 = -95541436;    int BEsWlsGYrh47459750 = -313984225;    int BEsWlsGYrh75541809 = -535151428;    int BEsWlsGYrh63096969 = -764620732;    int BEsWlsGYrh4753027 = -954443680;    int BEsWlsGYrh79228451 = -43570213;    int BEsWlsGYrh17294827 = -869706245;    int BEsWlsGYrh45095610 = -215867578;    int BEsWlsGYrh30146403 = -76237262;    int BEsWlsGYrh38838700 = -718824836;    int BEsWlsGYrh17772909 = 89627501;    int BEsWlsGYrh56512147 = -778615086;    int BEsWlsGYrh97773004 = -855685815;    int BEsWlsGYrh57162571 = -315346730;    int BEsWlsGYrh30820339 = -824846642;    int BEsWlsGYrh54842074 = 40587625;    int BEsWlsGYrh46172106 = -92683710;    int BEsWlsGYrh91648292 = -141923880;    int BEsWlsGYrh56222369 = -131232888;    int BEsWlsGYrh90755193 = -507967525;    int BEsWlsGYrh11340391 = -643175595;    int BEsWlsGYrh94640914 = -328610961;    int BEsWlsGYrh91200448 = 46270428;    int BEsWlsGYrh9171056 = -154190731;    int BEsWlsGYrh69696942 = -845659277;    int BEsWlsGYrh75428993 = -694123653;    int BEsWlsGYrh28067520 = -288058810;    int BEsWlsGYrh252825 = -84097821;    int BEsWlsGYrh64684652 = -462436118;    int BEsWlsGYrh23431371 = -250168404;    int BEsWlsGYrh14373705 = -944195725;    int BEsWlsGYrh16792267 = -284090676;    int BEsWlsGYrh197531 = -939155180;    int BEsWlsGYrh98110082 = 81433662;    int BEsWlsGYrh27916070 = -461711544;    int BEsWlsGYrh86524332 = -438604772;    int BEsWlsGYrh96702943 = -842665950;    int BEsWlsGYrh37425103 = -855233535;    int BEsWlsGYrh68854245 = 80888095;    int BEsWlsGYrh79256928 = -551331537;    int BEsWlsGYrh69815343 = -253211279;    int BEsWlsGYrh43339116 = 69830241;    int BEsWlsGYrh18136641 = -875690996;    int BEsWlsGYrh55067048 = -131153445;    int BEsWlsGYrh55919773 = -413725249;    int BEsWlsGYrh38538123 = -888469571;    int BEsWlsGYrh62027993 = -810660094;    int BEsWlsGYrh71505463 = -100377427;    int BEsWlsGYrh65694121 = -740321019;    int BEsWlsGYrh62280614 = -631531664;    int BEsWlsGYrh95695165 = -226809535;    int BEsWlsGYrh34638934 = -65866380;    int BEsWlsGYrh48250206 = -945898911;     BEsWlsGYrh95445045 = BEsWlsGYrh79499632;     BEsWlsGYrh79499632 = BEsWlsGYrh39037751;     BEsWlsGYrh39037751 = BEsWlsGYrh52383152;     BEsWlsGYrh52383152 = BEsWlsGYrh786130;     BEsWlsGYrh786130 = BEsWlsGYrh55280874;     BEsWlsGYrh55280874 = BEsWlsGYrh64804421;     BEsWlsGYrh64804421 = BEsWlsGYrh27086827;     BEsWlsGYrh27086827 = BEsWlsGYrh46694179;     BEsWlsGYrh46694179 = BEsWlsGYrh65255565;     BEsWlsGYrh65255565 = BEsWlsGYrh14707749;     BEsWlsGYrh14707749 = BEsWlsGYrh77865364;     BEsWlsGYrh77865364 = BEsWlsGYrh99775116;     BEsWlsGYrh99775116 = BEsWlsGYrh42923958;     BEsWlsGYrh42923958 = BEsWlsGYrh85106197;     BEsWlsGYrh85106197 = BEsWlsGYrh82863738;     BEsWlsGYrh82863738 = BEsWlsGYrh64747212;     BEsWlsGYrh64747212 = BEsWlsGYrh35229991;     BEsWlsGYrh35229991 = BEsWlsGYrh56735554;     BEsWlsGYrh56735554 = BEsWlsGYrh96767284;     BEsWlsGYrh96767284 = BEsWlsGYrh81807341;     BEsWlsGYrh81807341 = BEsWlsGYrh65298642;     BEsWlsGYrh65298642 = BEsWlsGYrh40660932;     BEsWlsGYrh40660932 = BEsWlsGYrh21264842;     BEsWlsGYrh21264842 = BEsWlsGYrh95871004;     BEsWlsGYrh95871004 = BEsWlsGYrh3013126;     BEsWlsGYrh3013126 = BEsWlsGYrh98118302;     BEsWlsGYrh98118302 = BEsWlsGYrh33984083;     BEsWlsGYrh33984083 = BEsWlsGYrh72244752;     BEsWlsGYrh72244752 = BEsWlsGYrh522073;     BEsWlsGYrh522073 = BEsWlsGYrh73607272;     BEsWlsGYrh73607272 = BEsWlsGYrh58485380;     BEsWlsGYrh58485380 = BEsWlsGYrh87110170;     BEsWlsGYrh87110170 = BEsWlsGYrh88434726;     BEsWlsGYrh88434726 = BEsWlsGYrh48283044;     BEsWlsGYrh48283044 = BEsWlsGYrh93905748;     BEsWlsGYrh93905748 = BEsWlsGYrh73692682;     BEsWlsGYrh73692682 = BEsWlsGYrh95050269;     BEsWlsGYrh95050269 = BEsWlsGYrh59800998;     BEsWlsGYrh59800998 = BEsWlsGYrh28668035;     BEsWlsGYrh28668035 = BEsWlsGYrh96514460;     BEsWlsGYrh96514460 = BEsWlsGYrh17122689;     BEsWlsGYrh17122689 = BEsWlsGYrh41867272;     BEsWlsGYrh41867272 = BEsWlsGYrh26287227;     BEsWlsGYrh26287227 = BEsWlsGYrh4472575;     BEsWlsGYrh4472575 = BEsWlsGYrh95673474;     BEsWlsGYrh95673474 = BEsWlsGYrh4903044;     BEsWlsGYrh4903044 = BEsWlsGYrh70202233;     BEsWlsGYrh70202233 = BEsWlsGYrh47459750;     BEsWlsGYrh47459750 = BEsWlsGYrh75541809;     BEsWlsGYrh75541809 = BEsWlsGYrh63096969;     BEsWlsGYrh63096969 = BEsWlsGYrh4753027;     BEsWlsGYrh4753027 = BEsWlsGYrh79228451;     BEsWlsGYrh79228451 = BEsWlsGYrh17294827;     BEsWlsGYrh17294827 = BEsWlsGYrh45095610;     BEsWlsGYrh45095610 = BEsWlsGYrh30146403;     BEsWlsGYrh30146403 = BEsWlsGYrh38838700;     BEsWlsGYrh38838700 = BEsWlsGYrh17772909;     BEsWlsGYrh17772909 = BEsWlsGYrh56512147;     BEsWlsGYrh56512147 = BEsWlsGYrh97773004;     BEsWlsGYrh97773004 = BEsWlsGYrh57162571;     BEsWlsGYrh57162571 = BEsWlsGYrh30820339;     BEsWlsGYrh30820339 = BEsWlsGYrh54842074;     BEsWlsGYrh54842074 = BEsWlsGYrh46172106;     BEsWlsGYrh46172106 = BEsWlsGYrh91648292;     BEsWlsGYrh91648292 = BEsWlsGYrh56222369;     BEsWlsGYrh56222369 = BEsWlsGYrh90755193;     BEsWlsGYrh90755193 = BEsWlsGYrh11340391;     BEsWlsGYrh11340391 = BEsWlsGYrh94640914;     BEsWlsGYrh94640914 = BEsWlsGYrh91200448;     BEsWlsGYrh91200448 = BEsWlsGYrh9171056;     BEsWlsGYrh9171056 = BEsWlsGYrh69696942;     BEsWlsGYrh69696942 = BEsWlsGYrh75428993;     BEsWlsGYrh75428993 = BEsWlsGYrh28067520;     BEsWlsGYrh28067520 = BEsWlsGYrh252825;     BEsWlsGYrh252825 = BEsWlsGYrh64684652;     BEsWlsGYrh64684652 = BEsWlsGYrh23431371;     BEsWlsGYrh23431371 = BEsWlsGYrh14373705;     BEsWlsGYrh14373705 = BEsWlsGYrh16792267;     BEsWlsGYrh16792267 = BEsWlsGYrh197531;     BEsWlsGYrh197531 = BEsWlsGYrh98110082;     BEsWlsGYrh98110082 = BEsWlsGYrh27916070;     BEsWlsGYrh27916070 = BEsWlsGYrh86524332;     BEsWlsGYrh86524332 = BEsWlsGYrh96702943;     BEsWlsGYrh96702943 = BEsWlsGYrh37425103;     BEsWlsGYrh37425103 = BEsWlsGYrh68854245;     BEsWlsGYrh68854245 = BEsWlsGYrh79256928;     BEsWlsGYrh79256928 = BEsWlsGYrh69815343;     BEsWlsGYrh69815343 = BEsWlsGYrh43339116;     BEsWlsGYrh43339116 = BEsWlsGYrh18136641;     BEsWlsGYrh18136641 = BEsWlsGYrh55067048;     BEsWlsGYrh55067048 = BEsWlsGYrh55919773;     BEsWlsGYrh55919773 = BEsWlsGYrh38538123;     BEsWlsGYrh38538123 = BEsWlsGYrh62027993;     BEsWlsGYrh62027993 = BEsWlsGYrh71505463;     BEsWlsGYrh71505463 = BEsWlsGYrh65694121;     BEsWlsGYrh65694121 = BEsWlsGYrh62280614;     BEsWlsGYrh62280614 = BEsWlsGYrh95695165;     BEsWlsGYrh95695165 = BEsWlsGYrh34638934;     BEsWlsGYrh34638934 = BEsWlsGYrh48250206;     BEsWlsGYrh48250206 = BEsWlsGYrh95445045;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DXlAlUqiDq97358824() {     int CYAklRCRlK8231213 = 19060398;    int CYAklRCRlK66270520 = -141144832;    int CYAklRCRlK56790550 = -577184873;    int CYAklRCRlK60323977 = -205349962;    int CYAklRCRlK94297234 = -761738178;    int CYAklRCRlK98590944 = -849523022;    int CYAklRCRlK86012179 = -123107709;    int CYAklRCRlK53805940 = -881748008;    int CYAklRCRlK42098989 = -774296068;    int CYAklRCRlK71474269 = 80432903;    int CYAklRCRlK42195131 = 59179347;    int CYAklRCRlK9815870 = -138217864;    int CYAklRCRlK69918419 = -892995588;    int CYAklRCRlK92548036 = -712954416;    int CYAklRCRlK8412751 = -910693683;    int CYAklRCRlK11288807 = -129179867;    int CYAklRCRlK20499030 = -176518400;    int CYAklRCRlK18788477 = 84320137;    int CYAklRCRlK57896185 = -813252161;    int CYAklRCRlK99421780 = -248509620;    int CYAklRCRlK61981398 = -644696420;    int CYAklRCRlK94346461 = -939519997;    int CYAklRCRlK79421227 = -182989878;    int CYAklRCRlK65771971 = -518798005;    int CYAklRCRlK1765265 = -812974524;    int CYAklRCRlK84331770 = -104667098;    int CYAklRCRlK9533839 = -190959260;    int CYAklRCRlK1337008 = 73612899;    int CYAklRCRlK88861805 = -592034617;    int CYAklRCRlK53273188 = -249740421;    int CYAklRCRlK18043268 = -149972335;    int CYAklRCRlK61804203 = -673621428;    int CYAklRCRlK33156120 = 1227692;    int CYAklRCRlK65511099 = -150791555;    int CYAklRCRlK31343576 = -561960138;    int CYAklRCRlK89129863 = -738541385;    int CYAklRCRlK10456743 = -214007098;    int CYAklRCRlK52754292 = -586704748;    int CYAklRCRlK65576384 = -508196486;    int CYAklRCRlK54932764 = -360141002;    int CYAklRCRlK57945768 = -940641654;    int CYAklRCRlK24825145 = -589255373;    int CYAklRCRlK76750742 = -442926690;    int CYAklRCRlK70915764 = -648962502;    int CYAklRCRlK84481065 = -767132277;    int CYAklRCRlK69301604 = -736695860;    int CYAklRCRlK82485063 = -893184613;    int CYAklRCRlK99642199 = -250894216;    int CYAklRCRlK91523799 = -477586200;    int CYAklRCRlK22492230 = -46187891;    int CYAklRCRlK81424462 = 82494959;    int CYAklRCRlK32929574 = -325629018;    int CYAklRCRlK13713602 = -219573496;    int CYAklRCRlK61994552 = -459157987;    int CYAklRCRlK35377426 = -210312064;    int CYAklRCRlK13884752 = -41419606;    int CYAklRCRlK86849293 = -958154954;    int CYAklRCRlK91018579 = 41613132;    int CYAklRCRlK58558713 = -392375438;    int CYAklRCRlK9965464 = -557071081;    int CYAklRCRlK89057105 = -558563763;    int CYAklRCRlK84675172 = -96720608;    int CYAklRCRlK64944134 = -189713392;    int CYAklRCRlK88825801 = -424555648;    int CYAklRCRlK53431002 = -769594763;    int CYAklRCRlK80390927 = -267199225;    int CYAklRCRlK76659749 = -39445556;    int CYAklRCRlK4407321 = -642204034;    int CYAklRCRlK61204460 = -50994278;    int CYAklRCRlK19282887 = -72152298;    int CYAklRCRlK832064 = -915172770;    int CYAklRCRlK67744737 = -589813653;    int CYAklRCRlK53212092 = -407483378;    int CYAklRCRlK2963421 = -353111159;    int CYAklRCRlK41476013 = -307867967;    int CYAklRCRlK37156253 = 44558952;    int CYAklRCRlK17595720 = -396593307;    int CYAklRCRlK8505463 = -534027377;    int CYAklRCRlK81290905 = -751665728;    int CYAklRCRlK32463660 = 23721335;    int CYAklRCRlK1846708 = -211482485;    int CYAklRCRlK9891639 = -940065044;    int CYAklRCRlK9813209 = -448800902;    int CYAklRCRlK66369576 = -445846727;    int CYAklRCRlK71848725 = -232235381;    int CYAklRCRlK85113694 = -824343317;    int CYAklRCRlK48090602 = -354047933;    int CYAklRCRlK71161567 = -539614321;    int CYAklRCRlK30133673 = -940479491;    int CYAklRCRlK17458825 = -420540533;    int CYAklRCRlK2280571 = -780386431;    int CYAklRCRlK19438164 = -155620230;    int CYAklRCRlK94195579 = -94329310;    int CYAklRCRlK55610920 = -951125405;    int CYAklRCRlK65875658 = -801577240;    int CYAklRCRlK73270596 = -743921047;    int CYAklRCRlK59881011 = -299541982;    int CYAklRCRlK87924940 = 81628958;    int CYAklRCRlK17484763 = -879367740;    int CYAklRCRlK4090138 = 19060398;     CYAklRCRlK8231213 = CYAklRCRlK66270520;     CYAklRCRlK66270520 = CYAklRCRlK56790550;     CYAklRCRlK56790550 = CYAklRCRlK60323977;     CYAklRCRlK60323977 = CYAklRCRlK94297234;     CYAklRCRlK94297234 = CYAklRCRlK98590944;     CYAklRCRlK98590944 = CYAklRCRlK86012179;     CYAklRCRlK86012179 = CYAklRCRlK53805940;     CYAklRCRlK53805940 = CYAklRCRlK42098989;     CYAklRCRlK42098989 = CYAklRCRlK71474269;     CYAklRCRlK71474269 = CYAklRCRlK42195131;     CYAklRCRlK42195131 = CYAklRCRlK9815870;     CYAklRCRlK9815870 = CYAklRCRlK69918419;     CYAklRCRlK69918419 = CYAklRCRlK92548036;     CYAklRCRlK92548036 = CYAklRCRlK8412751;     CYAklRCRlK8412751 = CYAklRCRlK11288807;     CYAklRCRlK11288807 = CYAklRCRlK20499030;     CYAklRCRlK20499030 = CYAklRCRlK18788477;     CYAklRCRlK18788477 = CYAklRCRlK57896185;     CYAklRCRlK57896185 = CYAklRCRlK99421780;     CYAklRCRlK99421780 = CYAklRCRlK61981398;     CYAklRCRlK61981398 = CYAklRCRlK94346461;     CYAklRCRlK94346461 = CYAklRCRlK79421227;     CYAklRCRlK79421227 = CYAklRCRlK65771971;     CYAklRCRlK65771971 = CYAklRCRlK1765265;     CYAklRCRlK1765265 = CYAklRCRlK84331770;     CYAklRCRlK84331770 = CYAklRCRlK9533839;     CYAklRCRlK9533839 = CYAklRCRlK1337008;     CYAklRCRlK1337008 = CYAklRCRlK88861805;     CYAklRCRlK88861805 = CYAklRCRlK53273188;     CYAklRCRlK53273188 = CYAklRCRlK18043268;     CYAklRCRlK18043268 = CYAklRCRlK61804203;     CYAklRCRlK61804203 = CYAklRCRlK33156120;     CYAklRCRlK33156120 = CYAklRCRlK65511099;     CYAklRCRlK65511099 = CYAklRCRlK31343576;     CYAklRCRlK31343576 = CYAklRCRlK89129863;     CYAklRCRlK89129863 = CYAklRCRlK10456743;     CYAklRCRlK10456743 = CYAklRCRlK52754292;     CYAklRCRlK52754292 = CYAklRCRlK65576384;     CYAklRCRlK65576384 = CYAklRCRlK54932764;     CYAklRCRlK54932764 = CYAklRCRlK57945768;     CYAklRCRlK57945768 = CYAklRCRlK24825145;     CYAklRCRlK24825145 = CYAklRCRlK76750742;     CYAklRCRlK76750742 = CYAklRCRlK70915764;     CYAklRCRlK70915764 = CYAklRCRlK84481065;     CYAklRCRlK84481065 = CYAklRCRlK69301604;     CYAklRCRlK69301604 = CYAklRCRlK82485063;     CYAklRCRlK82485063 = CYAklRCRlK99642199;     CYAklRCRlK99642199 = CYAklRCRlK91523799;     CYAklRCRlK91523799 = CYAklRCRlK22492230;     CYAklRCRlK22492230 = CYAklRCRlK81424462;     CYAklRCRlK81424462 = CYAklRCRlK32929574;     CYAklRCRlK32929574 = CYAklRCRlK13713602;     CYAklRCRlK13713602 = CYAklRCRlK61994552;     CYAklRCRlK61994552 = CYAklRCRlK35377426;     CYAklRCRlK35377426 = CYAklRCRlK13884752;     CYAklRCRlK13884752 = CYAklRCRlK86849293;     CYAklRCRlK86849293 = CYAklRCRlK91018579;     CYAklRCRlK91018579 = CYAklRCRlK58558713;     CYAklRCRlK58558713 = CYAklRCRlK9965464;     CYAklRCRlK9965464 = CYAklRCRlK89057105;     CYAklRCRlK89057105 = CYAklRCRlK84675172;     CYAklRCRlK84675172 = CYAklRCRlK64944134;     CYAklRCRlK64944134 = CYAklRCRlK88825801;     CYAklRCRlK88825801 = CYAklRCRlK53431002;     CYAklRCRlK53431002 = CYAklRCRlK80390927;     CYAklRCRlK80390927 = CYAklRCRlK76659749;     CYAklRCRlK76659749 = CYAklRCRlK4407321;     CYAklRCRlK4407321 = CYAklRCRlK61204460;     CYAklRCRlK61204460 = CYAklRCRlK19282887;     CYAklRCRlK19282887 = CYAklRCRlK832064;     CYAklRCRlK832064 = CYAklRCRlK67744737;     CYAklRCRlK67744737 = CYAklRCRlK53212092;     CYAklRCRlK53212092 = CYAklRCRlK2963421;     CYAklRCRlK2963421 = CYAklRCRlK41476013;     CYAklRCRlK41476013 = CYAklRCRlK37156253;     CYAklRCRlK37156253 = CYAklRCRlK17595720;     CYAklRCRlK17595720 = CYAklRCRlK8505463;     CYAklRCRlK8505463 = CYAklRCRlK81290905;     CYAklRCRlK81290905 = CYAklRCRlK32463660;     CYAklRCRlK32463660 = CYAklRCRlK1846708;     CYAklRCRlK1846708 = CYAklRCRlK9891639;     CYAklRCRlK9891639 = CYAklRCRlK9813209;     CYAklRCRlK9813209 = CYAklRCRlK66369576;     CYAklRCRlK66369576 = CYAklRCRlK71848725;     CYAklRCRlK71848725 = CYAklRCRlK85113694;     CYAklRCRlK85113694 = CYAklRCRlK48090602;     CYAklRCRlK48090602 = CYAklRCRlK71161567;     CYAklRCRlK71161567 = CYAklRCRlK30133673;     CYAklRCRlK30133673 = CYAklRCRlK17458825;     CYAklRCRlK17458825 = CYAklRCRlK2280571;     CYAklRCRlK2280571 = CYAklRCRlK19438164;     CYAklRCRlK19438164 = CYAklRCRlK94195579;     CYAklRCRlK94195579 = CYAklRCRlK55610920;     CYAklRCRlK55610920 = CYAklRCRlK65875658;     CYAklRCRlK65875658 = CYAklRCRlK73270596;     CYAklRCRlK73270596 = CYAklRCRlK59881011;     CYAklRCRlK59881011 = CYAklRCRlK87924940;     CYAklRCRlK87924940 = CYAklRCRlK17484763;     CYAklRCRlK17484763 = CYAklRCRlK4090138;     CYAklRCRlK4090138 = CYAklRCRlK8231213;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fpvqeZxDam15572166() {     int jajZKeXJFX91675518 = -861605283;    int jajZKeXJFX83142609 = -140527020;    int jajZKeXJFX53199057 = -370710101;    int jajZKeXJFX23116229 = -399149798;    int jajZKeXJFX418559 = -568605669;    int jajZKeXJFX97095345 = -628594675;    int jajZKeXJFX57051497 = -963019022;    int jajZKeXJFX68153382 = -868906038;    int jajZKeXJFX628690 = -15755419;    int jajZKeXJFX4019922 = -387943209;    int jajZKeXJFX91611068 = -130936525;    int jajZKeXJFX73021268 = -125836110;    int jajZKeXJFX88036195 = -684656829;    int jajZKeXJFX72174928 = -501244773;    int jajZKeXJFX69267214 = 95283335;    int jajZKeXJFX97677968 = 82740808;    int jajZKeXJFX14250915 = 70136573;    int jajZKeXJFX99639922 = -162984739;    int jajZKeXJFX20336327 = -811927032;    int jajZKeXJFX88789223 = -778095613;    int jajZKeXJFX17621786 = -234225451;    int jajZKeXJFX6493780 = -930671767;    int jajZKeXJFX83246934 = -222879075;    int jajZKeXJFX478530 = -590623401;    int jajZKeXJFX43341222 = -272627283;    int jajZKeXJFX79673919 = 14823187;    int jajZKeXJFX58988579 = -919043602;    int jajZKeXJFX41068633 = 3658015;    int jajZKeXJFX24942900 = -454539760;    int jajZKeXJFX69106529 = -762590272;    int jajZKeXJFX36138582 = -887729536;    int jajZKeXJFX24910631 = -588816508;    int jajZKeXJFX76695006 = -719644486;    int jajZKeXJFX7669610 = -842393725;    int jajZKeXJFX10070307 = -759624874;    int jajZKeXJFX1519048 = -526565291;    int jajZKeXJFX79070921 = -92856278;    int jajZKeXJFX93333124 = -295175268;    int jajZKeXJFX14571462 = -693094283;    int jajZKeXJFX4427900 = -112734525;    int jajZKeXJFX91594692 = -512190628;    int jajZKeXJFX75312931 = -63163694;    int jajZKeXJFX317812 = -993983933;    int jajZKeXJFX62820705 = -405145273;    int jajZKeXJFX76425562 = -495289231;    int jajZKeXJFX8907600 = -756306783;    int jajZKeXJFX58500493 = 74511157;    int jajZKeXJFX19312795 = -249615934;    int jajZKeXJFX22370912 = 36811718;    int jajZKeXJFX57824713 = -253208346;    int jajZKeXJFX54681911 = -138406463;    int jajZKeXJFX88430368 = -383888330;    int jajZKeXJFX27826061 = -431320812;    int jajZKeXJFX86671311 = -177571375;    int jajZKeXJFX97950380 = -558798187;    int jajZKeXJFX85181738 = -930933517;    int jajZKeXJFX99895674 = -917647945;    int jajZKeXJFX52720528 = -780086700;    int jajZKeXJFX79775006 = -26522515;    int jajZKeXJFX20744639 = -483428856;    int jajZKeXJFX38106767 = -709551073;    int jajZKeXJFX15982865 = -866677037;    int jajZKeXJFX43210483 = -314366279;    int jajZKeXJFX31522161 = -253165147;    int jajZKeXJFX67881340 = -500213674;    int jajZKeXJFX66700438 = -542120018;    int jajZKeXJFX96326262 = -406191625;    int jajZKeXJFX80366586 = -842263105;    int jajZKeXJFX62104622 = -741619900;    int jajZKeXJFX67748166 = -378151375;    int jajZKeXJFX18607047 = -824402915;    int jajZKeXJFX20917790 = -634688160;    int jajZKeXJFX85068460 = -469890457;    int jajZKeXJFX15908428 = -599192508;    int jajZKeXJFX97194530 = -165904986;    int jajZKeXJFX42308854 = -71061757;    int jajZKeXJFX6175969 = -936687835;    int jajZKeXJFX20426230 = -817733803;    int jajZKeXJFX24052967 = 4665829;    int jajZKeXJFX34433622 = -516320500;    int jajZKeXJFX21173427 = 40312029;    int jajZKeXJFX39675784 = -569427668;    int jajZKeXJFX18697722 = 66846297;    int jajZKeXJFX67118186 = -101331414;    int jajZKeXJFX14424619 = -524183810;    int jajZKeXJFX47708214 = -403841206;    int jajZKeXJFX97084569 = -57495696;    int jajZKeXJFX90023695 = -442073111;    int jajZKeXJFX9719229 = -183595538;    int jajZKeXJFX24888568 = -828691357;    int jajZKeXJFX1623374 = -608917346;    int jajZKeXJFX26350394 = -312769578;    int jajZKeXJFX13558118 = -168652754;    int jajZKeXJFX93826823 = -109665427;    int jajZKeXJFX66321133 = -403183452;    int jajZKeXJFX75611828 = -645513591;    int jajZKeXJFX32102449 = -748797416;    int jajZKeXJFX68795650 = -640818786;    int jajZKeXJFX94939364 = -904931600;    int jajZKeXJFX9725124 = -861605283;     jajZKeXJFX91675518 = jajZKeXJFX83142609;     jajZKeXJFX83142609 = jajZKeXJFX53199057;     jajZKeXJFX53199057 = jajZKeXJFX23116229;     jajZKeXJFX23116229 = jajZKeXJFX418559;     jajZKeXJFX418559 = jajZKeXJFX97095345;     jajZKeXJFX97095345 = jajZKeXJFX57051497;     jajZKeXJFX57051497 = jajZKeXJFX68153382;     jajZKeXJFX68153382 = jajZKeXJFX628690;     jajZKeXJFX628690 = jajZKeXJFX4019922;     jajZKeXJFX4019922 = jajZKeXJFX91611068;     jajZKeXJFX91611068 = jajZKeXJFX73021268;     jajZKeXJFX73021268 = jajZKeXJFX88036195;     jajZKeXJFX88036195 = jajZKeXJFX72174928;     jajZKeXJFX72174928 = jajZKeXJFX69267214;     jajZKeXJFX69267214 = jajZKeXJFX97677968;     jajZKeXJFX97677968 = jajZKeXJFX14250915;     jajZKeXJFX14250915 = jajZKeXJFX99639922;     jajZKeXJFX99639922 = jajZKeXJFX20336327;     jajZKeXJFX20336327 = jajZKeXJFX88789223;     jajZKeXJFX88789223 = jajZKeXJFX17621786;     jajZKeXJFX17621786 = jajZKeXJFX6493780;     jajZKeXJFX6493780 = jajZKeXJFX83246934;     jajZKeXJFX83246934 = jajZKeXJFX478530;     jajZKeXJFX478530 = jajZKeXJFX43341222;     jajZKeXJFX43341222 = jajZKeXJFX79673919;     jajZKeXJFX79673919 = jajZKeXJFX58988579;     jajZKeXJFX58988579 = jajZKeXJFX41068633;     jajZKeXJFX41068633 = jajZKeXJFX24942900;     jajZKeXJFX24942900 = jajZKeXJFX69106529;     jajZKeXJFX69106529 = jajZKeXJFX36138582;     jajZKeXJFX36138582 = jajZKeXJFX24910631;     jajZKeXJFX24910631 = jajZKeXJFX76695006;     jajZKeXJFX76695006 = jajZKeXJFX7669610;     jajZKeXJFX7669610 = jajZKeXJFX10070307;     jajZKeXJFX10070307 = jajZKeXJFX1519048;     jajZKeXJFX1519048 = jajZKeXJFX79070921;     jajZKeXJFX79070921 = jajZKeXJFX93333124;     jajZKeXJFX93333124 = jajZKeXJFX14571462;     jajZKeXJFX14571462 = jajZKeXJFX4427900;     jajZKeXJFX4427900 = jajZKeXJFX91594692;     jajZKeXJFX91594692 = jajZKeXJFX75312931;     jajZKeXJFX75312931 = jajZKeXJFX317812;     jajZKeXJFX317812 = jajZKeXJFX62820705;     jajZKeXJFX62820705 = jajZKeXJFX76425562;     jajZKeXJFX76425562 = jajZKeXJFX8907600;     jajZKeXJFX8907600 = jajZKeXJFX58500493;     jajZKeXJFX58500493 = jajZKeXJFX19312795;     jajZKeXJFX19312795 = jajZKeXJFX22370912;     jajZKeXJFX22370912 = jajZKeXJFX57824713;     jajZKeXJFX57824713 = jajZKeXJFX54681911;     jajZKeXJFX54681911 = jajZKeXJFX88430368;     jajZKeXJFX88430368 = jajZKeXJFX27826061;     jajZKeXJFX27826061 = jajZKeXJFX86671311;     jajZKeXJFX86671311 = jajZKeXJFX97950380;     jajZKeXJFX97950380 = jajZKeXJFX85181738;     jajZKeXJFX85181738 = jajZKeXJFX99895674;     jajZKeXJFX99895674 = jajZKeXJFX52720528;     jajZKeXJFX52720528 = jajZKeXJFX79775006;     jajZKeXJFX79775006 = jajZKeXJFX20744639;     jajZKeXJFX20744639 = jajZKeXJFX38106767;     jajZKeXJFX38106767 = jajZKeXJFX15982865;     jajZKeXJFX15982865 = jajZKeXJFX43210483;     jajZKeXJFX43210483 = jajZKeXJFX31522161;     jajZKeXJFX31522161 = jajZKeXJFX67881340;     jajZKeXJFX67881340 = jajZKeXJFX66700438;     jajZKeXJFX66700438 = jajZKeXJFX96326262;     jajZKeXJFX96326262 = jajZKeXJFX80366586;     jajZKeXJFX80366586 = jajZKeXJFX62104622;     jajZKeXJFX62104622 = jajZKeXJFX67748166;     jajZKeXJFX67748166 = jajZKeXJFX18607047;     jajZKeXJFX18607047 = jajZKeXJFX20917790;     jajZKeXJFX20917790 = jajZKeXJFX85068460;     jajZKeXJFX85068460 = jajZKeXJFX15908428;     jajZKeXJFX15908428 = jajZKeXJFX97194530;     jajZKeXJFX97194530 = jajZKeXJFX42308854;     jajZKeXJFX42308854 = jajZKeXJFX6175969;     jajZKeXJFX6175969 = jajZKeXJFX20426230;     jajZKeXJFX20426230 = jajZKeXJFX24052967;     jajZKeXJFX24052967 = jajZKeXJFX34433622;     jajZKeXJFX34433622 = jajZKeXJFX21173427;     jajZKeXJFX21173427 = jajZKeXJFX39675784;     jajZKeXJFX39675784 = jajZKeXJFX18697722;     jajZKeXJFX18697722 = jajZKeXJFX67118186;     jajZKeXJFX67118186 = jajZKeXJFX14424619;     jajZKeXJFX14424619 = jajZKeXJFX47708214;     jajZKeXJFX47708214 = jajZKeXJFX97084569;     jajZKeXJFX97084569 = jajZKeXJFX90023695;     jajZKeXJFX90023695 = jajZKeXJFX9719229;     jajZKeXJFX9719229 = jajZKeXJFX24888568;     jajZKeXJFX24888568 = jajZKeXJFX1623374;     jajZKeXJFX1623374 = jajZKeXJFX26350394;     jajZKeXJFX26350394 = jajZKeXJFX13558118;     jajZKeXJFX13558118 = jajZKeXJFX93826823;     jajZKeXJFX93826823 = jajZKeXJFX66321133;     jajZKeXJFX66321133 = jajZKeXJFX75611828;     jajZKeXJFX75611828 = jajZKeXJFX32102449;     jajZKeXJFX32102449 = jajZKeXJFX68795650;     jajZKeXJFX68795650 = jajZKeXJFX94939364;     jajZKeXJFX94939364 = jajZKeXJFX9725124;     jajZKeXJFX9725124 = jajZKeXJFX91675518;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XBeDgDSupZ86028038() {     int ADTimpEyHP4461686 = -996645975;    int ADTimpEyHP69913497 = -310861033;    int ADTimpEyHP70951857 = -779069483;    int ADTimpEyHP31057054 = -223352074;    int ADTimpEyHP93929662 = 39577866;    int ADTimpEyHP40405416 = -405517989;    int ADTimpEyHP78259255 = -408691092;    int ADTimpEyHP94872495 = -213424295;    int ADTimpEyHP96033500 = 22486489;    int ADTimpEyHP10238627 = -192030842;    int ADTimpEyHP19098451 = -245622541;    int ADTimpEyHP4971774 = -633168926;    int ADTimpEyHP58179498 = -588439486;    int ADTimpEyHP21799007 = -833659972;    int ADTimpEyHP92573766 = -911702496;    int ADTimpEyHP26103038 = -468150582;    int ADTimpEyHP70002732 = -693637894;    int ADTimpEyHP83198407 = -818195042;    int ADTimpEyHP21496958 = -721396227;    int ADTimpEyHP91443719 = -277339752;    int ADTimpEyHP97795842 = -725541715;    int ADTimpEyHP35541599 = -530115;    int ADTimpEyHP22007230 = -153882970;    int ADTimpEyHP44985659 = -950968414;    int ADTimpEyHP49235481 = -483069207;    int ADTimpEyHP60992565 = -775608012;    int ADTimpEyHP70404115 = -452749882;    int ADTimpEyHP8421558 = -170140090;    int ADTimpEyHP41559953 = -668757000;    int ADTimpEyHP21857645 = -392476427;    int ADTimpEyHP80574577 = -64146286;    int ADTimpEyHP28229454 = -567536186;    int ADTimpEyHP22740956 = -595499270;    int ADTimpEyHP84745982 = -747147943;    int ADTimpEyHP93130838 = -269656754;    int ADTimpEyHP96743162 = -315128395;    int ADTimpEyHP15834982 = -982765628;    int ADTimpEyHP51037147 = -214795360;    int ADTimpEyHP20346848 = -534944860;    int ADTimpEyHP30692629 = 42848630;    int ADTimpEyHP53026001 = -887664621;    int ADTimpEyHP83015387 = 38524971;    int ADTimpEyHP35201281 = 82582623;    int ADTimpEyHP7449243 = -746317516;    int ADTimpEyHP56434052 = -388059191;    int ADTimpEyHP82535729 = -829625223;    int ADTimpEyHP36082513 = -423003895;    int ADTimpEyHP48752761 = -404968714;    int ADTimpEyHP66434961 = -126790257;    int ADTimpEyHP4775134 = -864244809;    int ADTimpEyHP73009404 = -391290771;    int ADTimpEyHP16606916 = -855073668;    int ADTimpEyHP62311211 = -607324095;    int ADTimpEyHP31371036 = -867023117;    int ADTimpEyHP88232196 = -553242674;    int ADTimpEyHP68920086 = -896115860;    int ADTimpEyHP47906267 = -56978064;    int ADTimpEyHP25966199 = -828101070;    int ADTimpEyHP81821573 = -740282868;    int ADTimpEyHP32937098 = -184814122;    int ADTimpEyHP70001301 = -952768107;    int ADTimpEyHP69837697 = -138551003;    int ADTimpEyHP53312542 = -544667295;    int ADTimpEyHP74175856 = -585037085;    int ADTimpEyHP29664049 = -27884557;    int ADTimpEyHP90868996 = -678086355;    int ADTimpEyHP82230818 = 62330344;    int ADTimpEyHP73433516 = -841291544;    int ADTimpEyHP28668169 = -464003218;    int ADTimpEyHP95830604 = -496574101;    int ADTimpEyHP10268056 = -485384954;    int ADTimpEyHP18965585 = -378842535;    int ADTimpEyHP62851560 = -183250182;    int ADTimpEyHP90804328 = -664244858;    int ADTimpEyHP38417719 = -389675132;    int ADTimpEyHP14780455 = -664066687;    int ADTimpEyHP340318 = 16887262;    int ADTimpEyHP14557988 = -407565455;    int ADTimpEyHP88551606 = -462909223;    int ADTimpEyHP66699752 = -653443985;    int ADTimpEyHP24910052 = -252604117;    int ADTimpEyHP21651354 = 52218831;    int ADTimpEyHP41986597 = 56650167;    int ADTimpEyHP36784819 = -804512191;    int ADTimpEyHP48848240 = 98814344;    int ADTimpEyHP63967662 = -209072618;    int ADTimpEyHP65918243 = -960212092;    int ADTimpEyHP91369919 = -728476154;    int ADTimpEyHP96513785 = -93905270;    int ADTimpEyHP24210752 = -373540894;    int ADTimpEyHP48836896 = -158150332;    int ADTimpEyHP89868783 = -54664559;    int ADTimpEyHP69215574 = -474512493;    int ADTimpEyHP87409749 = -250130739;    int ADTimpEyHP60691328 = -4383264;    int ADTimpEyHP83188303 = -649113618;    int ADTimpEyHP29702845 = -416807734;    int ADTimpEyHP61025425 = -332380293;    int ADTimpEyHP77785194 = -618432960;    int ADTimpEyHP65565055 = -996645975;     ADTimpEyHP4461686 = ADTimpEyHP69913497;     ADTimpEyHP69913497 = ADTimpEyHP70951857;     ADTimpEyHP70951857 = ADTimpEyHP31057054;     ADTimpEyHP31057054 = ADTimpEyHP93929662;     ADTimpEyHP93929662 = ADTimpEyHP40405416;     ADTimpEyHP40405416 = ADTimpEyHP78259255;     ADTimpEyHP78259255 = ADTimpEyHP94872495;     ADTimpEyHP94872495 = ADTimpEyHP96033500;     ADTimpEyHP96033500 = ADTimpEyHP10238627;     ADTimpEyHP10238627 = ADTimpEyHP19098451;     ADTimpEyHP19098451 = ADTimpEyHP4971774;     ADTimpEyHP4971774 = ADTimpEyHP58179498;     ADTimpEyHP58179498 = ADTimpEyHP21799007;     ADTimpEyHP21799007 = ADTimpEyHP92573766;     ADTimpEyHP92573766 = ADTimpEyHP26103038;     ADTimpEyHP26103038 = ADTimpEyHP70002732;     ADTimpEyHP70002732 = ADTimpEyHP83198407;     ADTimpEyHP83198407 = ADTimpEyHP21496958;     ADTimpEyHP21496958 = ADTimpEyHP91443719;     ADTimpEyHP91443719 = ADTimpEyHP97795842;     ADTimpEyHP97795842 = ADTimpEyHP35541599;     ADTimpEyHP35541599 = ADTimpEyHP22007230;     ADTimpEyHP22007230 = ADTimpEyHP44985659;     ADTimpEyHP44985659 = ADTimpEyHP49235481;     ADTimpEyHP49235481 = ADTimpEyHP60992565;     ADTimpEyHP60992565 = ADTimpEyHP70404115;     ADTimpEyHP70404115 = ADTimpEyHP8421558;     ADTimpEyHP8421558 = ADTimpEyHP41559953;     ADTimpEyHP41559953 = ADTimpEyHP21857645;     ADTimpEyHP21857645 = ADTimpEyHP80574577;     ADTimpEyHP80574577 = ADTimpEyHP28229454;     ADTimpEyHP28229454 = ADTimpEyHP22740956;     ADTimpEyHP22740956 = ADTimpEyHP84745982;     ADTimpEyHP84745982 = ADTimpEyHP93130838;     ADTimpEyHP93130838 = ADTimpEyHP96743162;     ADTimpEyHP96743162 = ADTimpEyHP15834982;     ADTimpEyHP15834982 = ADTimpEyHP51037147;     ADTimpEyHP51037147 = ADTimpEyHP20346848;     ADTimpEyHP20346848 = ADTimpEyHP30692629;     ADTimpEyHP30692629 = ADTimpEyHP53026001;     ADTimpEyHP53026001 = ADTimpEyHP83015387;     ADTimpEyHP83015387 = ADTimpEyHP35201281;     ADTimpEyHP35201281 = ADTimpEyHP7449243;     ADTimpEyHP7449243 = ADTimpEyHP56434052;     ADTimpEyHP56434052 = ADTimpEyHP82535729;     ADTimpEyHP82535729 = ADTimpEyHP36082513;     ADTimpEyHP36082513 = ADTimpEyHP48752761;     ADTimpEyHP48752761 = ADTimpEyHP66434961;     ADTimpEyHP66434961 = ADTimpEyHP4775134;     ADTimpEyHP4775134 = ADTimpEyHP73009404;     ADTimpEyHP73009404 = ADTimpEyHP16606916;     ADTimpEyHP16606916 = ADTimpEyHP62311211;     ADTimpEyHP62311211 = ADTimpEyHP31371036;     ADTimpEyHP31371036 = ADTimpEyHP88232196;     ADTimpEyHP88232196 = ADTimpEyHP68920086;     ADTimpEyHP68920086 = ADTimpEyHP47906267;     ADTimpEyHP47906267 = ADTimpEyHP25966199;     ADTimpEyHP25966199 = ADTimpEyHP81821573;     ADTimpEyHP81821573 = ADTimpEyHP32937098;     ADTimpEyHP32937098 = ADTimpEyHP70001301;     ADTimpEyHP70001301 = ADTimpEyHP69837697;     ADTimpEyHP69837697 = ADTimpEyHP53312542;     ADTimpEyHP53312542 = ADTimpEyHP74175856;     ADTimpEyHP74175856 = ADTimpEyHP29664049;     ADTimpEyHP29664049 = ADTimpEyHP90868996;     ADTimpEyHP90868996 = ADTimpEyHP82230818;     ADTimpEyHP82230818 = ADTimpEyHP73433516;     ADTimpEyHP73433516 = ADTimpEyHP28668169;     ADTimpEyHP28668169 = ADTimpEyHP95830604;     ADTimpEyHP95830604 = ADTimpEyHP10268056;     ADTimpEyHP10268056 = ADTimpEyHP18965585;     ADTimpEyHP18965585 = ADTimpEyHP62851560;     ADTimpEyHP62851560 = ADTimpEyHP90804328;     ADTimpEyHP90804328 = ADTimpEyHP38417719;     ADTimpEyHP38417719 = ADTimpEyHP14780455;     ADTimpEyHP14780455 = ADTimpEyHP340318;     ADTimpEyHP340318 = ADTimpEyHP14557988;     ADTimpEyHP14557988 = ADTimpEyHP88551606;     ADTimpEyHP88551606 = ADTimpEyHP66699752;     ADTimpEyHP66699752 = ADTimpEyHP24910052;     ADTimpEyHP24910052 = ADTimpEyHP21651354;     ADTimpEyHP21651354 = ADTimpEyHP41986597;     ADTimpEyHP41986597 = ADTimpEyHP36784819;     ADTimpEyHP36784819 = ADTimpEyHP48848240;     ADTimpEyHP48848240 = ADTimpEyHP63967662;     ADTimpEyHP63967662 = ADTimpEyHP65918243;     ADTimpEyHP65918243 = ADTimpEyHP91369919;     ADTimpEyHP91369919 = ADTimpEyHP96513785;     ADTimpEyHP96513785 = ADTimpEyHP24210752;     ADTimpEyHP24210752 = ADTimpEyHP48836896;     ADTimpEyHP48836896 = ADTimpEyHP89868783;     ADTimpEyHP89868783 = ADTimpEyHP69215574;     ADTimpEyHP69215574 = ADTimpEyHP87409749;     ADTimpEyHP87409749 = ADTimpEyHP60691328;     ADTimpEyHP60691328 = ADTimpEyHP83188303;     ADTimpEyHP83188303 = ADTimpEyHP29702845;     ADTimpEyHP29702845 = ADTimpEyHP61025425;     ADTimpEyHP61025425 = ADTimpEyHP77785194;     ADTimpEyHP77785194 = ADTimpEyHP65565055;     ADTimpEyHP65565055 = ADTimpEyHP4461686;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xhsiBEWbif86470141() {     int KbXIgprCbc39641441 = -893566488;    int KbXIgprCbc2459570 = -387142976;    int KbXIgprCbc16160724 = -369954095;    int KbXIgprCbc68924707 = -233421053;    int KbXIgprCbc76774919 = -574940279;    int KbXIgprCbc63793171 = -343616868;    int KbXIgprCbc23075417 = 28185998;    int KbXIgprCbc28011416 = -324361878;    int KbXIgprCbc5861278 = -109821573;    int KbXIgprCbc11581403 = -102052937;    int KbXIgprCbc63807087 = -397460885;    int KbXIgprCbc78533550 = -555768673;    int KbXIgprCbc63478068 = 10719011;    int KbXIgprCbc60193618 = -248630875;    int KbXIgprCbc7443488 = -483453188;    int KbXIgprCbc24219472 = 32086137;    int KbXIgprCbc19725142 = -815077611;    int KbXIgprCbc49732776 = -428076413;    int KbXIgprCbc43510949 = -147985281;    int KbXIgprCbc540737 = -517193893;    int KbXIgprCbc99183581 = -397878914;    int KbXIgprCbc50108371 = -165162893;    int KbXIgprCbc27182452 = -902009615;    int KbXIgprCbc72342467 = -894385761;    int KbXIgprCbc62227298 = -988376403;    int KbXIgprCbc14040128 = 98272493;    int KbXIgprCbc58687490 = -543242943;    int KbXIgprCbc46282408 = -623425660;    int KbXIgprCbc52391120 = -581161044;    int KbXIgprCbc7676070 = -267227075;    int KbXIgprCbc35888700 = -34785953;    int KbXIgprCbc4365612 = -806505458;    int KbXIgprCbc64373152 = -798753333;    int KbXIgprCbc20928206 = 652721;    int KbXIgprCbc70062697 = -684131132;    int KbXIgprCbc90831957 = -339321129;    int KbXIgprCbc34097387 = -405969552;    int KbXIgprCbc92449591 = -6778244;    int KbXIgprCbc39116769 = -65160053;    int KbXIgprCbc5270181 = -328360559;    int KbXIgprCbc55359012 = -578372382;    int KbXIgprCbc93528573 = -523902972;    int KbXIgprCbc15351583 = -369251152;    int KbXIgprCbc92290001 = -427888964;    int KbXIgprCbc20407757 = -716713227;    int KbXIgprCbc23836173 = -173128087;    int KbXIgprCbc98264137 = 82351422;    int KbXIgprCbc47407821 = -808095128;    int KbXIgprCbc86300526 = -676345069;    int KbXIgprCbc28763877 = -930276644;    int KbXIgprCbc86946744 = -320696349;    int KbXIgprCbc4087463 = -274932540;    int KbXIgprCbc99662415 = -563184599;    int KbXIgprCbc26107036 = -200235816;    int KbXIgprCbc87286560 = -931491998;    int KbXIgprCbc89533070 = -628403596;    int KbXIgprCbc75277117 = -485133362;    int KbXIgprCbc43818257 = -475568335;    int KbXIgprCbc6697410 = -245044650;    int KbXIgprCbc62734792 = -573212772;    int KbXIgprCbc5105682 = -800373926;    int KbXIgprCbc76793008 = -348388343;    int KbXIgprCbc75620296 = -743200835;    int KbXIgprCbc98185208 = -842594499;    int KbXIgprCbc75692702 = 32733016;    int KbXIgprCbc59441476 = -590955428;    int KbXIgprCbc14160399 = -757015340;    int KbXIgprCbc42549862 = -989933710;    int KbXIgprCbc90130920 = -564499744;    int KbXIgprCbc16611531 = -44132059;    int KbXIgprCbc90122084 = -561944311;    int KbXIgprCbc27275551 = -708299367;    int KbXIgprCbc10616008 = -262916360;    int KbXIgprCbc38240768 = -819624723;    int KbXIgprCbc45181724 = -938821512;    int KbXIgprCbc5655009 = -873975943;    int KbXIgprCbc34756788 = -795911741;    int KbXIgprCbc34892451 = -374120651;    int KbXIgprCbc51934710 = -77672534;    int KbXIgprCbc38391126 = -715248316;    int KbXIgprCbc15775991 = -984078929;    int KbXIgprCbc11279669 = -735147815;    int KbXIgprCbc59981882 = -947080592;    int KbXIgprCbc23627243 = -650884400;    int KbXIgprCbc20729325 = -946530726;    int KbXIgprCbc31801238 = -759853414;    int KbXIgprCbc4703196 = -143320859;    int KbXIgprCbc38266116 = -498517518;    int KbXIgprCbc33641646 = -67855282;    int KbXIgprCbc80529626 = 44272463;    int KbXIgprCbc15554840 = -854187768;    int KbXIgprCbc90279130 = -930401218;    int KbXIgprCbc85752181 = -761733595;    int KbXIgprCbc76381977 = -491947281;    int KbXIgprCbc164500 = -527986634;    int KbXIgprCbc78566003 = -129984039;    int KbXIgprCbc17908277 = -780702137;    int KbXIgprCbc17166375 = -526656654;    int KbXIgprCbc16597300 = -360621981;    int KbXIgprCbc60966281 = -893566488;     KbXIgprCbc39641441 = KbXIgprCbc2459570;     KbXIgprCbc2459570 = KbXIgprCbc16160724;     KbXIgprCbc16160724 = KbXIgprCbc68924707;     KbXIgprCbc68924707 = KbXIgprCbc76774919;     KbXIgprCbc76774919 = KbXIgprCbc63793171;     KbXIgprCbc63793171 = KbXIgprCbc23075417;     KbXIgprCbc23075417 = KbXIgprCbc28011416;     KbXIgprCbc28011416 = KbXIgprCbc5861278;     KbXIgprCbc5861278 = KbXIgprCbc11581403;     KbXIgprCbc11581403 = KbXIgprCbc63807087;     KbXIgprCbc63807087 = KbXIgprCbc78533550;     KbXIgprCbc78533550 = KbXIgprCbc63478068;     KbXIgprCbc63478068 = KbXIgprCbc60193618;     KbXIgprCbc60193618 = KbXIgprCbc7443488;     KbXIgprCbc7443488 = KbXIgprCbc24219472;     KbXIgprCbc24219472 = KbXIgprCbc19725142;     KbXIgprCbc19725142 = KbXIgprCbc49732776;     KbXIgprCbc49732776 = KbXIgprCbc43510949;     KbXIgprCbc43510949 = KbXIgprCbc540737;     KbXIgprCbc540737 = KbXIgprCbc99183581;     KbXIgprCbc99183581 = KbXIgprCbc50108371;     KbXIgprCbc50108371 = KbXIgprCbc27182452;     KbXIgprCbc27182452 = KbXIgprCbc72342467;     KbXIgprCbc72342467 = KbXIgprCbc62227298;     KbXIgprCbc62227298 = KbXIgprCbc14040128;     KbXIgprCbc14040128 = KbXIgprCbc58687490;     KbXIgprCbc58687490 = KbXIgprCbc46282408;     KbXIgprCbc46282408 = KbXIgprCbc52391120;     KbXIgprCbc52391120 = KbXIgprCbc7676070;     KbXIgprCbc7676070 = KbXIgprCbc35888700;     KbXIgprCbc35888700 = KbXIgprCbc4365612;     KbXIgprCbc4365612 = KbXIgprCbc64373152;     KbXIgprCbc64373152 = KbXIgprCbc20928206;     KbXIgprCbc20928206 = KbXIgprCbc70062697;     KbXIgprCbc70062697 = KbXIgprCbc90831957;     KbXIgprCbc90831957 = KbXIgprCbc34097387;     KbXIgprCbc34097387 = KbXIgprCbc92449591;     KbXIgprCbc92449591 = KbXIgprCbc39116769;     KbXIgprCbc39116769 = KbXIgprCbc5270181;     KbXIgprCbc5270181 = KbXIgprCbc55359012;     KbXIgprCbc55359012 = KbXIgprCbc93528573;     KbXIgprCbc93528573 = KbXIgprCbc15351583;     KbXIgprCbc15351583 = KbXIgprCbc92290001;     KbXIgprCbc92290001 = KbXIgprCbc20407757;     KbXIgprCbc20407757 = KbXIgprCbc23836173;     KbXIgprCbc23836173 = KbXIgprCbc98264137;     KbXIgprCbc98264137 = KbXIgprCbc47407821;     KbXIgprCbc47407821 = KbXIgprCbc86300526;     KbXIgprCbc86300526 = KbXIgprCbc28763877;     KbXIgprCbc28763877 = KbXIgprCbc86946744;     KbXIgprCbc86946744 = KbXIgprCbc4087463;     KbXIgprCbc4087463 = KbXIgprCbc99662415;     KbXIgprCbc99662415 = KbXIgprCbc26107036;     KbXIgprCbc26107036 = KbXIgprCbc87286560;     KbXIgprCbc87286560 = KbXIgprCbc89533070;     KbXIgprCbc89533070 = KbXIgprCbc75277117;     KbXIgprCbc75277117 = KbXIgprCbc43818257;     KbXIgprCbc43818257 = KbXIgprCbc6697410;     KbXIgprCbc6697410 = KbXIgprCbc62734792;     KbXIgprCbc62734792 = KbXIgprCbc5105682;     KbXIgprCbc5105682 = KbXIgprCbc76793008;     KbXIgprCbc76793008 = KbXIgprCbc75620296;     KbXIgprCbc75620296 = KbXIgprCbc98185208;     KbXIgprCbc98185208 = KbXIgprCbc75692702;     KbXIgprCbc75692702 = KbXIgprCbc59441476;     KbXIgprCbc59441476 = KbXIgprCbc14160399;     KbXIgprCbc14160399 = KbXIgprCbc42549862;     KbXIgprCbc42549862 = KbXIgprCbc90130920;     KbXIgprCbc90130920 = KbXIgprCbc16611531;     KbXIgprCbc16611531 = KbXIgprCbc90122084;     KbXIgprCbc90122084 = KbXIgprCbc27275551;     KbXIgprCbc27275551 = KbXIgprCbc10616008;     KbXIgprCbc10616008 = KbXIgprCbc38240768;     KbXIgprCbc38240768 = KbXIgprCbc45181724;     KbXIgprCbc45181724 = KbXIgprCbc5655009;     KbXIgprCbc5655009 = KbXIgprCbc34756788;     KbXIgprCbc34756788 = KbXIgprCbc34892451;     KbXIgprCbc34892451 = KbXIgprCbc51934710;     KbXIgprCbc51934710 = KbXIgprCbc38391126;     KbXIgprCbc38391126 = KbXIgprCbc15775991;     KbXIgprCbc15775991 = KbXIgprCbc11279669;     KbXIgprCbc11279669 = KbXIgprCbc59981882;     KbXIgprCbc59981882 = KbXIgprCbc23627243;     KbXIgprCbc23627243 = KbXIgprCbc20729325;     KbXIgprCbc20729325 = KbXIgprCbc31801238;     KbXIgprCbc31801238 = KbXIgprCbc4703196;     KbXIgprCbc4703196 = KbXIgprCbc38266116;     KbXIgprCbc38266116 = KbXIgprCbc33641646;     KbXIgprCbc33641646 = KbXIgprCbc80529626;     KbXIgprCbc80529626 = KbXIgprCbc15554840;     KbXIgprCbc15554840 = KbXIgprCbc90279130;     KbXIgprCbc90279130 = KbXIgprCbc85752181;     KbXIgprCbc85752181 = KbXIgprCbc76381977;     KbXIgprCbc76381977 = KbXIgprCbc164500;     KbXIgprCbc164500 = KbXIgprCbc78566003;     KbXIgprCbc78566003 = KbXIgprCbc17908277;     KbXIgprCbc17908277 = KbXIgprCbc17166375;     KbXIgprCbc17166375 = KbXIgprCbc16597300;     KbXIgprCbc16597300 = KbXIgprCbc60966281;     KbXIgprCbc60966281 = KbXIgprCbc39641441;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kKggdTkpSL4683484() {     int xqAbhvNuga23085747 = -674232169;    int xqAbhvNuga19331659 = -386525164;    int xqAbhvNuga12569231 = -163479323;    int xqAbhvNuga31716959 = -427220888;    int xqAbhvNuga82896243 = -381807770;    int xqAbhvNuga62297572 = -122688521;    int xqAbhvNuga94114734 = -811725315;    int xqAbhvNuga42358859 = -311519908;    int xqAbhvNuga64390978 = -451280924;    int xqAbhvNuga44127054 = -570429049;    int xqAbhvNuga13223025 = -587576758;    int xqAbhvNuga41738950 = -543386919;    int xqAbhvNuga81595843 = -880942230;    int xqAbhvNuga39820510 = -36921232;    int xqAbhvNuga68297951 = -577476170;    int xqAbhvNuga10608633 = -855993188;    int xqAbhvNuga13477027 = -568422638;    int xqAbhvNuga30584223 = -675381289;    int xqAbhvNuga5951091 = -146660152;    int xqAbhvNuga89908178 = 53220114;    int xqAbhvNuga54823969 = 12592055;    int xqAbhvNuga62255689 = -156314663;    int xqAbhvNuga31008160 = -941898812;    int xqAbhvNuga7049026 = -966211157;    int xqAbhvNuga3803256 = -448029162;    int xqAbhvNuga9382277 = -882237223;    int xqAbhvNuga8142231 = -171327285;    int xqAbhvNuga86014033 = -693380544;    int xqAbhvNuga88472213 = -443666186;    int xqAbhvNuga23509411 = -780076926;    int xqAbhvNuga53984014 = -772543154;    int xqAbhvNuga67472038 = -721700538;    int xqAbhvNuga7912039 = -419625511;    int xqAbhvNuga63086716 = -690949449;    int xqAbhvNuga48789427 = -881795867;    int xqAbhvNuga3221142 = -127345035;    int xqAbhvNuga2711566 = -284818732;    int xqAbhvNuga33028423 = -815248764;    int xqAbhvNuga88111846 = -250057851;    int xqAbhvNuga54765316 = -80954081;    int xqAbhvNuga89007936 = -149921355;    int xqAbhvNuga44016359 = 2188707;    int xqAbhvNuga38918653 = -920308395;    int xqAbhvNuga84194941 = -184071735;    int xqAbhvNuga12352254 = -444870180;    int xqAbhvNuga63442168 = -192739011;    int xqAbhvNuga74279567 = -49952808;    int xqAbhvNuga67078417 = -806816846;    int xqAbhvNuga17147639 = -161947151;    int xqAbhvNuga64096361 = -37297100;    int xqAbhvNuga60204192 = -541597772;    int xqAbhvNuga59588256 = -333191851;    int xqAbhvNuga13774875 = -774931916;    int xqAbhvNuga50783795 = 81350797;    int xqAbhvNuga49859514 = -179978121;    int xqAbhvNuga60830058 = -417917506;    int xqAbhvNuga88323498 = -444626353;    int xqAbhvNuga5520206 = -197268167;    int xqAbhvNuga27913703 = -979191727;    int xqAbhvNuga73513966 = -499570548;    int xqAbhvNuga54155342 = -951361237;    int xqAbhvNuga8100701 = -18344772;    int xqAbhvNuga53886645 = -867853722;    int xqAbhvNuga40881568 = -671203998;    int xqAbhvNuga90143040 = -797885895;    int xqAbhvNuga45750987 = -865876220;    int xqAbhvNuga33826911 = -23761409;    int xqAbhvNuga18509128 = -89992781;    int xqAbhvNuga91031082 = -155125365;    int xqAbhvNuga65076810 = -350131135;    int xqAbhvNuga7897068 = -471174457;    int xqAbhvNuga80448603 = -753173874;    int xqAbhvNuga42472376 = -325323439;    int xqAbhvNuga51185775 = 34293929;    int xqAbhvNuga900243 = -796858531;    int xqAbhvNuga10807610 = -989596652;    int xqAbhvNuga23337036 = -236006269;    int xqAbhvNuga46813218 = -657827077;    int xqAbhvNuga94696771 = -421340977;    int xqAbhvNuga40361088 = -155290151;    int xqAbhvNuga35102709 = -732284415;    int xqAbhvNuga41063813 = -364510439;    int xqAbhvNuga68866395 = -431433393;    int xqAbhvNuga24375853 = -306369087;    int xqAbhvNuga63305218 = -138479155;    int xqAbhvNuga94395757 = -339351304;    int xqAbhvNuga53697164 = -946768623;    int xqAbhvNuga57128244 = -400976308;    int xqAbhvNuga13227202 = -410971329;    int xqAbhvNuga87959369 = -363878362;    int xqAbhvNuga14897643 = -682718683;    int xqAbhvNuga97191360 = 12449434;    int xqAbhvNuga5114721 = -836057038;    int xqAbhvNuga14597880 = -750487304;    int xqAbhvNuga609974 = -129592845;    int xqAbhvNuga80907235 = -31576583;    int xqAbhvNuga90129714 = -129957572;    int xqAbhvNuga98037084 = -149104397;    int xqAbhvNuga94051901 = -386185840;    int xqAbhvNuga66601267 = -674232169;     xqAbhvNuga23085747 = xqAbhvNuga19331659;     xqAbhvNuga19331659 = xqAbhvNuga12569231;     xqAbhvNuga12569231 = xqAbhvNuga31716959;     xqAbhvNuga31716959 = xqAbhvNuga82896243;     xqAbhvNuga82896243 = xqAbhvNuga62297572;     xqAbhvNuga62297572 = xqAbhvNuga94114734;     xqAbhvNuga94114734 = xqAbhvNuga42358859;     xqAbhvNuga42358859 = xqAbhvNuga64390978;     xqAbhvNuga64390978 = xqAbhvNuga44127054;     xqAbhvNuga44127054 = xqAbhvNuga13223025;     xqAbhvNuga13223025 = xqAbhvNuga41738950;     xqAbhvNuga41738950 = xqAbhvNuga81595843;     xqAbhvNuga81595843 = xqAbhvNuga39820510;     xqAbhvNuga39820510 = xqAbhvNuga68297951;     xqAbhvNuga68297951 = xqAbhvNuga10608633;     xqAbhvNuga10608633 = xqAbhvNuga13477027;     xqAbhvNuga13477027 = xqAbhvNuga30584223;     xqAbhvNuga30584223 = xqAbhvNuga5951091;     xqAbhvNuga5951091 = xqAbhvNuga89908178;     xqAbhvNuga89908178 = xqAbhvNuga54823969;     xqAbhvNuga54823969 = xqAbhvNuga62255689;     xqAbhvNuga62255689 = xqAbhvNuga31008160;     xqAbhvNuga31008160 = xqAbhvNuga7049026;     xqAbhvNuga7049026 = xqAbhvNuga3803256;     xqAbhvNuga3803256 = xqAbhvNuga9382277;     xqAbhvNuga9382277 = xqAbhvNuga8142231;     xqAbhvNuga8142231 = xqAbhvNuga86014033;     xqAbhvNuga86014033 = xqAbhvNuga88472213;     xqAbhvNuga88472213 = xqAbhvNuga23509411;     xqAbhvNuga23509411 = xqAbhvNuga53984014;     xqAbhvNuga53984014 = xqAbhvNuga67472038;     xqAbhvNuga67472038 = xqAbhvNuga7912039;     xqAbhvNuga7912039 = xqAbhvNuga63086716;     xqAbhvNuga63086716 = xqAbhvNuga48789427;     xqAbhvNuga48789427 = xqAbhvNuga3221142;     xqAbhvNuga3221142 = xqAbhvNuga2711566;     xqAbhvNuga2711566 = xqAbhvNuga33028423;     xqAbhvNuga33028423 = xqAbhvNuga88111846;     xqAbhvNuga88111846 = xqAbhvNuga54765316;     xqAbhvNuga54765316 = xqAbhvNuga89007936;     xqAbhvNuga89007936 = xqAbhvNuga44016359;     xqAbhvNuga44016359 = xqAbhvNuga38918653;     xqAbhvNuga38918653 = xqAbhvNuga84194941;     xqAbhvNuga84194941 = xqAbhvNuga12352254;     xqAbhvNuga12352254 = xqAbhvNuga63442168;     xqAbhvNuga63442168 = xqAbhvNuga74279567;     xqAbhvNuga74279567 = xqAbhvNuga67078417;     xqAbhvNuga67078417 = xqAbhvNuga17147639;     xqAbhvNuga17147639 = xqAbhvNuga64096361;     xqAbhvNuga64096361 = xqAbhvNuga60204192;     xqAbhvNuga60204192 = xqAbhvNuga59588256;     xqAbhvNuga59588256 = xqAbhvNuga13774875;     xqAbhvNuga13774875 = xqAbhvNuga50783795;     xqAbhvNuga50783795 = xqAbhvNuga49859514;     xqAbhvNuga49859514 = xqAbhvNuga60830058;     xqAbhvNuga60830058 = xqAbhvNuga88323498;     xqAbhvNuga88323498 = xqAbhvNuga5520206;     xqAbhvNuga5520206 = xqAbhvNuga27913703;     xqAbhvNuga27913703 = xqAbhvNuga73513966;     xqAbhvNuga73513966 = xqAbhvNuga54155342;     xqAbhvNuga54155342 = xqAbhvNuga8100701;     xqAbhvNuga8100701 = xqAbhvNuga53886645;     xqAbhvNuga53886645 = xqAbhvNuga40881568;     xqAbhvNuga40881568 = xqAbhvNuga90143040;     xqAbhvNuga90143040 = xqAbhvNuga45750987;     xqAbhvNuga45750987 = xqAbhvNuga33826911;     xqAbhvNuga33826911 = xqAbhvNuga18509128;     xqAbhvNuga18509128 = xqAbhvNuga91031082;     xqAbhvNuga91031082 = xqAbhvNuga65076810;     xqAbhvNuga65076810 = xqAbhvNuga7897068;     xqAbhvNuga7897068 = xqAbhvNuga80448603;     xqAbhvNuga80448603 = xqAbhvNuga42472376;     xqAbhvNuga42472376 = xqAbhvNuga51185775;     xqAbhvNuga51185775 = xqAbhvNuga900243;     xqAbhvNuga900243 = xqAbhvNuga10807610;     xqAbhvNuga10807610 = xqAbhvNuga23337036;     xqAbhvNuga23337036 = xqAbhvNuga46813218;     xqAbhvNuga46813218 = xqAbhvNuga94696771;     xqAbhvNuga94696771 = xqAbhvNuga40361088;     xqAbhvNuga40361088 = xqAbhvNuga35102709;     xqAbhvNuga35102709 = xqAbhvNuga41063813;     xqAbhvNuga41063813 = xqAbhvNuga68866395;     xqAbhvNuga68866395 = xqAbhvNuga24375853;     xqAbhvNuga24375853 = xqAbhvNuga63305218;     xqAbhvNuga63305218 = xqAbhvNuga94395757;     xqAbhvNuga94395757 = xqAbhvNuga53697164;     xqAbhvNuga53697164 = xqAbhvNuga57128244;     xqAbhvNuga57128244 = xqAbhvNuga13227202;     xqAbhvNuga13227202 = xqAbhvNuga87959369;     xqAbhvNuga87959369 = xqAbhvNuga14897643;     xqAbhvNuga14897643 = xqAbhvNuga97191360;     xqAbhvNuga97191360 = xqAbhvNuga5114721;     xqAbhvNuga5114721 = xqAbhvNuga14597880;     xqAbhvNuga14597880 = xqAbhvNuga609974;     xqAbhvNuga609974 = xqAbhvNuga80907235;     xqAbhvNuga80907235 = xqAbhvNuga90129714;     xqAbhvNuga90129714 = xqAbhvNuga98037084;     xqAbhvNuga98037084 = xqAbhvNuga94051901;     xqAbhvNuga94051901 = xqAbhvNuga66601267;     xqAbhvNuga66601267 = xqAbhvNuga23085747;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LYJORViHLo75139356() {     int JCBGFwaArP35871914 = -809272861;    int JCBGFwaArP6102547 = -556859178;    int JCBGFwaArP30322031 = -571838704;    int JCBGFwaArP39657785 = -251423165;    int JCBGFwaArP76407348 = -873624235;    int JCBGFwaArP5607643 = -999611835;    int JCBGFwaArP15322493 = -257397386;    int JCBGFwaArP69077971 = -756038165;    int JCBGFwaArP59795789 = -413039016;    int JCBGFwaArP50345759 = -374516682;    int JCBGFwaArP40710407 = -702262773;    int JCBGFwaArP73689454 = 49280265;    int JCBGFwaArP51739147 = -784724887;    int JCBGFwaArP89444588 = -369336431;    int JCBGFwaArP91604504 = -484462001;    int JCBGFwaArP39033702 = -306884577;    int JCBGFwaArP69228844 = -232197105;    int JCBGFwaArP14142708 = -230591591;    int JCBGFwaArP7111722 = -56129348;    int JCBGFwaArP92562675 = -546024025;    int JCBGFwaArP34998026 = -478724209;    int JCBGFwaArP91303508 = -326173011;    int JCBGFwaArP69768455 = -872902707;    int JCBGFwaArP51556155 = -226556169;    int JCBGFwaArP9697516 = -658471086;    int JCBGFwaArP90700922 = -572668422;    int JCBGFwaArP19557767 = -805033565;    int JCBGFwaArP53366959 = -867178648;    int JCBGFwaArP5089267 = -657883426;    int JCBGFwaArP76260526 = -409963081;    int JCBGFwaArP98420009 = 51040096;    int JCBGFwaArP70790862 = -700420216;    int JCBGFwaArP53957988 = -295480295;    int JCBGFwaArP40163089 = -595703667;    int JCBGFwaArP31849960 = -391827748;    int JCBGFwaArP98445256 = 84091861;    int JCBGFwaArP39475626 = -74728082;    int JCBGFwaArP90732446 = -734868856;    int JCBGFwaArP93887232 = -91908428;    int JCBGFwaArP81030045 = 74629073;    int JCBGFwaArP50439245 = -525395348;    int JCBGFwaArP51718815 = -996122627;    int JCBGFwaArP73802122 = -943741840;    int JCBGFwaArP28823480 = -525243978;    int JCBGFwaArP92360744 = -337640141;    int JCBGFwaArP37070298 = -266057450;    int JCBGFwaArP51861587 = -547467860;    int JCBGFwaArP96518383 = -962169626;    int JCBGFwaArP61211688 = -325549126;    int JCBGFwaArP11046782 = -648333563;    int JCBGFwaArP78531686 = -794482080;    int JCBGFwaArP87764803 = -804377189;    int JCBGFwaArP48260025 = -950935198;    int JCBGFwaArP95483520 = -608100945;    int JCBGFwaArP40141331 = -174422607;    int JCBGFwaArP44568406 = -383099850;    int JCBGFwaArP36334091 = -683956471;    int JCBGFwaArP78765875 = -245282536;    int JCBGFwaArP29960270 = -592952080;    int JCBGFwaArP85706425 = -200955814;    int JCBGFwaArP86049876 = -94578270;    int JCBGFwaArP61955534 = -390218738;    int JCBGFwaArP63988704 = 1845261;    int JCBGFwaArP83535263 = 96924064;    int JCBGFwaArP51925749 = -325556778;    int JCBGFwaArP69919545 = 98157443;    int JCBGFwaArP19731467 = -655239440;    int JCBGFwaArP11576058 = -89021220;    int JCBGFwaArP57594629 = -977508683;    int JCBGFwaArP93159248 = -468553862;    int JCBGFwaArP99558075 = -132156495;    int JCBGFwaArP78496398 = -497328250;    int JCBGFwaArP20255475 = -38683164;    int JCBGFwaArP26081676 = -30758421;    int JCBGFwaArP42123431 = 79371323;    int JCBGFwaArP83279210 = -482601582;    int JCBGFwaArP17501386 = -382431172;    int JCBGFwaArP40944976 = -247658729;    int JCBGFwaArP59195411 = -888916029;    int JCBGFwaArP72627217 = -292413636;    int JCBGFwaArP38839335 = 74799438;    int JCBGFwaArP23039383 = -842863940;    int JCBGFwaArP92155270 = -441629523;    int JCBGFwaArP94042485 = 90450136;    int JCBGFwaArP97728839 = -615481001;    int JCBGFwaArP10655206 = -144582715;    int JCBGFwaArP22530837 = -749485018;    int JCBGFwaArP58474468 = -687379350;    int JCBGFwaArP21759 = -321281060;    int JCBGFwaArP87281553 = 91272102;    int JCBGFwaArP62111165 = -231951668;    int JCBGFwaArP60709750 = -829445547;    int JCBGFwaArP60772177 = -41916777;    int JCBGFwaArP8180807 = -890952615;    int JCBGFwaArP94980169 = -830792658;    int JCBGFwaArP88483710 = -35176611;    int JCBGFwaArP87730111 = -897967889;    int JCBGFwaArP90266859 = -940665904;    int JCBGFwaArP76897730 = -99687201;    int JCBGFwaArP22441199 = -809272861;     JCBGFwaArP35871914 = JCBGFwaArP6102547;     JCBGFwaArP6102547 = JCBGFwaArP30322031;     JCBGFwaArP30322031 = JCBGFwaArP39657785;     JCBGFwaArP39657785 = JCBGFwaArP76407348;     JCBGFwaArP76407348 = JCBGFwaArP5607643;     JCBGFwaArP5607643 = JCBGFwaArP15322493;     JCBGFwaArP15322493 = JCBGFwaArP69077971;     JCBGFwaArP69077971 = JCBGFwaArP59795789;     JCBGFwaArP59795789 = JCBGFwaArP50345759;     JCBGFwaArP50345759 = JCBGFwaArP40710407;     JCBGFwaArP40710407 = JCBGFwaArP73689454;     JCBGFwaArP73689454 = JCBGFwaArP51739147;     JCBGFwaArP51739147 = JCBGFwaArP89444588;     JCBGFwaArP89444588 = JCBGFwaArP91604504;     JCBGFwaArP91604504 = JCBGFwaArP39033702;     JCBGFwaArP39033702 = JCBGFwaArP69228844;     JCBGFwaArP69228844 = JCBGFwaArP14142708;     JCBGFwaArP14142708 = JCBGFwaArP7111722;     JCBGFwaArP7111722 = JCBGFwaArP92562675;     JCBGFwaArP92562675 = JCBGFwaArP34998026;     JCBGFwaArP34998026 = JCBGFwaArP91303508;     JCBGFwaArP91303508 = JCBGFwaArP69768455;     JCBGFwaArP69768455 = JCBGFwaArP51556155;     JCBGFwaArP51556155 = JCBGFwaArP9697516;     JCBGFwaArP9697516 = JCBGFwaArP90700922;     JCBGFwaArP90700922 = JCBGFwaArP19557767;     JCBGFwaArP19557767 = JCBGFwaArP53366959;     JCBGFwaArP53366959 = JCBGFwaArP5089267;     JCBGFwaArP5089267 = JCBGFwaArP76260526;     JCBGFwaArP76260526 = JCBGFwaArP98420009;     JCBGFwaArP98420009 = JCBGFwaArP70790862;     JCBGFwaArP70790862 = JCBGFwaArP53957988;     JCBGFwaArP53957988 = JCBGFwaArP40163089;     JCBGFwaArP40163089 = JCBGFwaArP31849960;     JCBGFwaArP31849960 = JCBGFwaArP98445256;     JCBGFwaArP98445256 = JCBGFwaArP39475626;     JCBGFwaArP39475626 = JCBGFwaArP90732446;     JCBGFwaArP90732446 = JCBGFwaArP93887232;     JCBGFwaArP93887232 = JCBGFwaArP81030045;     JCBGFwaArP81030045 = JCBGFwaArP50439245;     JCBGFwaArP50439245 = JCBGFwaArP51718815;     JCBGFwaArP51718815 = JCBGFwaArP73802122;     JCBGFwaArP73802122 = JCBGFwaArP28823480;     JCBGFwaArP28823480 = JCBGFwaArP92360744;     JCBGFwaArP92360744 = JCBGFwaArP37070298;     JCBGFwaArP37070298 = JCBGFwaArP51861587;     JCBGFwaArP51861587 = JCBGFwaArP96518383;     JCBGFwaArP96518383 = JCBGFwaArP61211688;     JCBGFwaArP61211688 = JCBGFwaArP11046782;     JCBGFwaArP11046782 = JCBGFwaArP78531686;     JCBGFwaArP78531686 = JCBGFwaArP87764803;     JCBGFwaArP87764803 = JCBGFwaArP48260025;     JCBGFwaArP48260025 = JCBGFwaArP95483520;     JCBGFwaArP95483520 = JCBGFwaArP40141331;     JCBGFwaArP40141331 = JCBGFwaArP44568406;     JCBGFwaArP44568406 = JCBGFwaArP36334091;     JCBGFwaArP36334091 = JCBGFwaArP78765875;     JCBGFwaArP78765875 = JCBGFwaArP29960270;     JCBGFwaArP29960270 = JCBGFwaArP85706425;     JCBGFwaArP85706425 = JCBGFwaArP86049876;     JCBGFwaArP86049876 = JCBGFwaArP61955534;     JCBGFwaArP61955534 = JCBGFwaArP63988704;     JCBGFwaArP63988704 = JCBGFwaArP83535263;     JCBGFwaArP83535263 = JCBGFwaArP51925749;     JCBGFwaArP51925749 = JCBGFwaArP69919545;     JCBGFwaArP69919545 = JCBGFwaArP19731467;     JCBGFwaArP19731467 = JCBGFwaArP11576058;     JCBGFwaArP11576058 = JCBGFwaArP57594629;     JCBGFwaArP57594629 = JCBGFwaArP93159248;     JCBGFwaArP93159248 = JCBGFwaArP99558075;     JCBGFwaArP99558075 = JCBGFwaArP78496398;     JCBGFwaArP78496398 = JCBGFwaArP20255475;     JCBGFwaArP20255475 = JCBGFwaArP26081676;     JCBGFwaArP26081676 = JCBGFwaArP42123431;     JCBGFwaArP42123431 = JCBGFwaArP83279210;     JCBGFwaArP83279210 = JCBGFwaArP17501386;     JCBGFwaArP17501386 = JCBGFwaArP40944976;     JCBGFwaArP40944976 = JCBGFwaArP59195411;     JCBGFwaArP59195411 = JCBGFwaArP72627217;     JCBGFwaArP72627217 = JCBGFwaArP38839335;     JCBGFwaArP38839335 = JCBGFwaArP23039383;     JCBGFwaArP23039383 = JCBGFwaArP92155270;     JCBGFwaArP92155270 = JCBGFwaArP94042485;     JCBGFwaArP94042485 = JCBGFwaArP97728839;     JCBGFwaArP97728839 = JCBGFwaArP10655206;     JCBGFwaArP10655206 = JCBGFwaArP22530837;     JCBGFwaArP22530837 = JCBGFwaArP58474468;     JCBGFwaArP58474468 = JCBGFwaArP21759;     JCBGFwaArP21759 = JCBGFwaArP87281553;     JCBGFwaArP87281553 = JCBGFwaArP62111165;     JCBGFwaArP62111165 = JCBGFwaArP60709750;     JCBGFwaArP60709750 = JCBGFwaArP60772177;     JCBGFwaArP60772177 = JCBGFwaArP8180807;     JCBGFwaArP8180807 = JCBGFwaArP94980169;     JCBGFwaArP94980169 = JCBGFwaArP88483710;     JCBGFwaArP88483710 = JCBGFwaArP87730111;     JCBGFwaArP87730111 = JCBGFwaArP90266859;     JCBGFwaArP90266859 = JCBGFwaArP76897730;     JCBGFwaArP76897730 = JCBGFwaArP22441199;     JCBGFwaArP22441199 = JCBGFwaArP35871914;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VdckZAqRbh93352697() {     int GyDjyCDiqv19316220 = -589938541;    int GyDjyCDiqv22974636 = -556241365;    int GyDjyCDiqv26730537 = -365363932;    int GyDjyCDiqv2450037 = -445223001;    int GyDjyCDiqv82528671 = -680491726;    int GyDjyCDiqv4112044 = -778683488;    int GyDjyCDiqv86361810 = 2691301;    int GyDjyCDiqv83425414 = -743196194;    int GyDjyCDiqv18325490 = -754498367;    int GyDjyCDiqv82891411 = -842892794;    int GyDjyCDiqv90126345 = -892378646;    int GyDjyCDiqv36894854 = 61662019;    int GyDjyCDiqv69856922 = -576386128;    int GyDjyCDiqv69071480 = -157626787;    int GyDjyCDiqv52458968 = -578484983;    int GyDjyCDiqv25422864 = -94963902;    int GyDjyCDiqv62980729 = 14457868;    int GyDjyCDiqv94994154 = -477896468;    int GyDjyCDiqv69551863 = -54804218;    int GyDjyCDiqv81930117 = 24389983;    int GyDjyCDiqv90638413 = -68253240;    int GyDjyCDiqv3450827 = -317324781;    int GyDjyCDiqv73594163 = -912791904;    int GyDjyCDiqv86262713 = -298381565;    int GyDjyCDiqv51273473 = -118123845;    int GyDjyCDiqv86043071 = -453178137;    int GyDjyCDiqv69012506 = -433117908;    int GyDjyCDiqv93098584 = -937133532;    int GyDjyCDiqv41170361 = -520388569;    int GyDjyCDiqv92093867 = -922812932;    int GyDjyCDiqv16515324 = -686717105;    int GyDjyCDiqv33897289 = -615615296;    int GyDjyCDiqv97496874 = 83647528;    int GyDjyCDiqv82321599 = -187305837;    int GyDjyCDiqv10576690 = -589492483;    int GyDjyCDiqv10834441 = -803932045;    int GyDjyCDiqv8089805 = 46422738;    int GyDjyCDiqv31311278 = -443339376;    int GyDjyCDiqv42882311 = -276806226;    int GyDjyCDiqv30525181 = -777964449;    int GyDjyCDiqv84088169 = -96944322;    int GyDjyCDiqv2206602 = -470030949;    int GyDjyCDiqv97369192 = -394799082;    int GyDjyCDiqv20728420 = -281426749;    int GyDjyCDiqv84305240 = -65797094;    int GyDjyCDiqv76676293 = -285668374;    int GyDjyCDiqv27877017 = -679772090;    int GyDjyCDiqv16188979 = -960891345;    int GyDjyCDiqv92058800 = -911151208;    int GyDjyCDiqv46379266 = -855354018;    int GyDjyCDiqv51789134 = 84616498;    int GyDjyCDiqv43265598 = -862636501;    int GyDjyCDiqv62372483 = -62682515;    int GyDjyCDiqv20160279 = -326514333;    int GyDjyCDiqv2714285 = -522908730;    int GyDjyCDiqv15865394 = -172613761;    int GyDjyCDiqv49380473 = -643449462;    int GyDjyCDiqv40467824 = 33017632;    int GyDjyCDiqv51176563 = -227099157;    int GyDjyCDiqv96485600 = -127313589;    int GyDjyCDiqv35099537 = -245565580;    int GyDjyCDiqv93263226 = -60175168;    int GyDjyCDiqv42255053 = -122807626;    int GyDjyCDiqv26231623 = -831685435;    int GyDjyCDiqv66376087 = -56175689;    int GyDjyCDiqv56229056 = -176763350;    int GyDjyCDiqv39397979 = 78014491;    int GyDjyCDiqv87535323 = -289080291;    int GyDjyCDiqv58494791 = -568134305;    int GyDjyCDiqv41624527 = -774552938;    int GyDjyCDiqv17333059 = -41386641;    int GyDjyCDiqv31669451 = -542202757;    int GyDjyCDiqv52111843 = -101090243;    int GyDjyCDiqv39026683 = -276839769;    int GyDjyCDiqv97841948 = -878665696;    int GyDjyCDiqv88431811 = -598222292;    int GyDjyCDiqv6081634 = -922525700;    int GyDjyCDiqv52865743 = -531365156;    int GyDjyCDiqv1957473 = -132584472;    int GyDjyCDiqv74597179 = -832455471;    int GyDjyCDiqv58166054 = -773406048;    int GyDjyCDiqv52823528 = -472226564;    int GyDjyCDiqv1039784 = 74017676;    int GyDjyCDiqv94791095 = -665034551;    int GyDjyCDiqv40304733 = -907429430;    int GyDjyCDiqv73249725 = -824080605;    int GyDjyCDiqv71524805 = -452932782;    int GyDjyCDiqv77336595 = -589838140;    int GyDjyCDiqv79607314 = -664397108;    int GyDjyCDiqv94711296 = -316878723;    int GyDjyCDiqv61453968 = -60482584;    int GyDjyCDiqv67621980 = -986594895;    int GyDjyCDiqv80134715 = -116240220;    int GyDjyCDiqv46396710 = -49492637;    int GyDjyCDiqv95425643 = -432398869;    int GyDjyCDiqv90824942 = 63230845;    int GyDjyCDiqv59951548 = -247223324;    int GyDjyCDiqv71137570 = -563113647;    int GyDjyCDiqv54352332 = -125251060;    int GyDjyCDiqv28076185 = -589938541;     GyDjyCDiqv19316220 = GyDjyCDiqv22974636;     GyDjyCDiqv22974636 = GyDjyCDiqv26730537;     GyDjyCDiqv26730537 = GyDjyCDiqv2450037;     GyDjyCDiqv2450037 = GyDjyCDiqv82528671;     GyDjyCDiqv82528671 = GyDjyCDiqv4112044;     GyDjyCDiqv4112044 = GyDjyCDiqv86361810;     GyDjyCDiqv86361810 = GyDjyCDiqv83425414;     GyDjyCDiqv83425414 = GyDjyCDiqv18325490;     GyDjyCDiqv18325490 = GyDjyCDiqv82891411;     GyDjyCDiqv82891411 = GyDjyCDiqv90126345;     GyDjyCDiqv90126345 = GyDjyCDiqv36894854;     GyDjyCDiqv36894854 = GyDjyCDiqv69856922;     GyDjyCDiqv69856922 = GyDjyCDiqv69071480;     GyDjyCDiqv69071480 = GyDjyCDiqv52458968;     GyDjyCDiqv52458968 = GyDjyCDiqv25422864;     GyDjyCDiqv25422864 = GyDjyCDiqv62980729;     GyDjyCDiqv62980729 = GyDjyCDiqv94994154;     GyDjyCDiqv94994154 = GyDjyCDiqv69551863;     GyDjyCDiqv69551863 = GyDjyCDiqv81930117;     GyDjyCDiqv81930117 = GyDjyCDiqv90638413;     GyDjyCDiqv90638413 = GyDjyCDiqv3450827;     GyDjyCDiqv3450827 = GyDjyCDiqv73594163;     GyDjyCDiqv73594163 = GyDjyCDiqv86262713;     GyDjyCDiqv86262713 = GyDjyCDiqv51273473;     GyDjyCDiqv51273473 = GyDjyCDiqv86043071;     GyDjyCDiqv86043071 = GyDjyCDiqv69012506;     GyDjyCDiqv69012506 = GyDjyCDiqv93098584;     GyDjyCDiqv93098584 = GyDjyCDiqv41170361;     GyDjyCDiqv41170361 = GyDjyCDiqv92093867;     GyDjyCDiqv92093867 = GyDjyCDiqv16515324;     GyDjyCDiqv16515324 = GyDjyCDiqv33897289;     GyDjyCDiqv33897289 = GyDjyCDiqv97496874;     GyDjyCDiqv97496874 = GyDjyCDiqv82321599;     GyDjyCDiqv82321599 = GyDjyCDiqv10576690;     GyDjyCDiqv10576690 = GyDjyCDiqv10834441;     GyDjyCDiqv10834441 = GyDjyCDiqv8089805;     GyDjyCDiqv8089805 = GyDjyCDiqv31311278;     GyDjyCDiqv31311278 = GyDjyCDiqv42882311;     GyDjyCDiqv42882311 = GyDjyCDiqv30525181;     GyDjyCDiqv30525181 = GyDjyCDiqv84088169;     GyDjyCDiqv84088169 = GyDjyCDiqv2206602;     GyDjyCDiqv2206602 = GyDjyCDiqv97369192;     GyDjyCDiqv97369192 = GyDjyCDiqv20728420;     GyDjyCDiqv20728420 = GyDjyCDiqv84305240;     GyDjyCDiqv84305240 = GyDjyCDiqv76676293;     GyDjyCDiqv76676293 = GyDjyCDiqv27877017;     GyDjyCDiqv27877017 = GyDjyCDiqv16188979;     GyDjyCDiqv16188979 = GyDjyCDiqv92058800;     GyDjyCDiqv92058800 = GyDjyCDiqv46379266;     GyDjyCDiqv46379266 = GyDjyCDiqv51789134;     GyDjyCDiqv51789134 = GyDjyCDiqv43265598;     GyDjyCDiqv43265598 = GyDjyCDiqv62372483;     GyDjyCDiqv62372483 = GyDjyCDiqv20160279;     GyDjyCDiqv20160279 = GyDjyCDiqv2714285;     GyDjyCDiqv2714285 = GyDjyCDiqv15865394;     GyDjyCDiqv15865394 = GyDjyCDiqv49380473;     GyDjyCDiqv49380473 = GyDjyCDiqv40467824;     GyDjyCDiqv40467824 = GyDjyCDiqv51176563;     GyDjyCDiqv51176563 = GyDjyCDiqv96485600;     GyDjyCDiqv96485600 = GyDjyCDiqv35099537;     GyDjyCDiqv35099537 = GyDjyCDiqv93263226;     GyDjyCDiqv93263226 = GyDjyCDiqv42255053;     GyDjyCDiqv42255053 = GyDjyCDiqv26231623;     GyDjyCDiqv26231623 = GyDjyCDiqv66376087;     GyDjyCDiqv66376087 = GyDjyCDiqv56229056;     GyDjyCDiqv56229056 = GyDjyCDiqv39397979;     GyDjyCDiqv39397979 = GyDjyCDiqv87535323;     GyDjyCDiqv87535323 = GyDjyCDiqv58494791;     GyDjyCDiqv58494791 = GyDjyCDiqv41624527;     GyDjyCDiqv41624527 = GyDjyCDiqv17333059;     GyDjyCDiqv17333059 = GyDjyCDiqv31669451;     GyDjyCDiqv31669451 = GyDjyCDiqv52111843;     GyDjyCDiqv52111843 = GyDjyCDiqv39026683;     GyDjyCDiqv39026683 = GyDjyCDiqv97841948;     GyDjyCDiqv97841948 = GyDjyCDiqv88431811;     GyDjyCDiqv88431811 = GyDjyCDiqv6081634;     GyDjyCDiqv6081634 = GyDjyCDiqv52865743;     GyDjyCDiqv52865743 = GyDjyCDiqv1957473;     GyDjyCDiqv1957473 = GyDjyCDiqv74597179;     GyDjyCDiqv74597179 = GyDjyCDiqv58166054;     GyDjyCDiqv58166054 = GyDjyCDiqv52823528;     GyDjyCDiqv52823528 = GyDjyCDiqv1039784;     GyDjyCDiqv1039784 = GyDjyCDiqv94791095;     GyDjyCDiqv94791095 = GyDjyCDiqv40304733;     GyDjyCDiqv40304733 = GyDjyCDiqv73249725;     GyDjyCDiqv73249725 = GyDjyCDiqv71524805;     GyDjyCDiqv71524805 = GyDjyCDiqv77336595;     GyDjyCDiqv77336595 = GyDjyCDiqv79607314;     GyDjyCDiqv79607314 = GyDjyCDiqv94711296;     GyDjyCDiqv94711296 = GyDjyCDiqv61453968;     GyDjyCDiqv61453968 = GyDjyCDiqv67621980;     GyDjyCDiqv67621980 = GyDjyCDiqv80134715;     GyDjyCDiqv80134715 = GyDjyCDiqv46396710;     GyDjyCDiqv46396710 = GyDjyCDiqv95425643;     GyDjyCDiqv95425643 = GyDjyCDiqv90824942;     GyDjyCDiqv90824942 = GyDjyCDiqv59951548;     GyDjyCDiqv59951548 = GyDjyCDiqv71137570;     GyDjyCDiqv71137570 = GyDjyCDiqv54352332;     GyDjyCDiqv54352332 = GyDjyCDiqv28076185;     GyDjyCDiqv28076185 = GyDjyCDiqv19316220;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XarqFFlafF63808571() {     int CarxOuIuIi32102387 = -724979233;    int CarxOuIuIi9745524 = -726575379;    int CarxOuIuIi44483337 = -773723314;    int CarxOuIuIi10390862 = -269425278;    int CarxOuIuIi76039776 = -72308191;    int CarxOuIuIi47422114 = -555606801;    int CarxOuIuIi7569569 = -542980770;    int CarxOuIuIi10144527 = -87714451;    int CarxOuIuIi13730300 = -716256459;    int CarxOuIuIi89110116 = -646980427;    int CarxOuIuIi17613728 = 92935339;    int CarxOuIuIi68845359 = -445670797;    int CarxOuIuIi40000225 = -480168785;    int CarxOuIuIi18695559 = -490041986;    int CarxOuIuIi75765521 = -485470813;    int CarxOuIuIi53847932 = -645855292;    int CarxOuIuIi18732547 = -749316599;    int CarxOuIuIi78552639 = -33106770;    int CarxOuIuIi70712494 = 35726586;    int CarxOuIuIi84584614 = -574854156;    int CarxOuIuIi70812470 = -559569505;    int CarxOuIuIi32498645 = -487183129;    int CarxOuIuIi12354459 = -843795799;    int CarxOuIuIi30769843 = -658726577;    int CarxOuIuIi57167732 = -328565769;    int CarxOuIuIi67361716 = -143609336;    int CarxOuIuIi80428042 = 33175812;    int CarxOuIuIi60451509 = -10931637;    int CarxOuIuIi57787414 = -734605809;    int CarxOuIuIi44844983 = -552699087;    int CarxOuIuIi60951319 = -963133855;    int CarxOuIuIi37216113 = -594334974;    int CarxOuIuIi43542824 = -892207257;    int CarxOuIuIi59397972 = -92060055;    int CarxOuIuIi93637222 = -99524363;    int CarxOuIuIi6058556 = -592495149;    int CarxOuIuIi44853865 = -843486613;    int CarxOuIuIi89015301 = -362959468;    int CarxOuIuIi48657697 = -118656803;    int CarxOuIuIi56789910 = -622381295;    int CarxOuIuIi45519477 = -472418315;    int CarxOuIuIi9909058 = -368342283;    int CarxOuIuIi32252662 = -418232527;    int CarxOuIuIi65356957 = -622598992;    int CarxOuIuIi64313731 = 41432946;    int CarxOuIuIi50304423 = -358986814;    int CarxOuIuIi5459037 = -77287142;    int CarxOuIuIi45628945 = -16244124;    int CarxOuIuIi36122850 = 25246817;    int CarxOuIuIi93329686 = -366390481;    int CarxOuIuIi70116627 = -168267811;    int CarxOuIuIi71442145 = -233821839;    int CarxOuIuIi96857633 = -238685797;    int CarxOuIuIi64860004 = 84033925;    int CarxOuIuIi92996101 = -517353217;    int CarxOuIuIi99603741 = -137796104;    int CarxOuIuIi97391065 = -882779581;    int CarxOuIuIi13713495 = -14996737;    int CarxOuIuIi53223129 = -940859509;    int CarxOuIuIi8678060 = -928698855;    int CarxOuIuIi66994071 = -488782614;    int CarxOuIuIi47118060 = -432049133;    int CarxOuIuIi52357113 = -353108642;    int CarxOuIuIi68885317 = -63557373;    int CarxOuIuIi28158797 = -683846572;    int CarxOuIuIi80397615 = -312729687;    int CarxOuIuIi25302535 = -553463541;    int CarxOuIuIi80602253 = -288108730;    int CarxOuIuIi25058337 = -290517623;    int CarxOuIuIi69706965 = -892975665;    int CarxOuIuIi8994068 = -802368679;    int CarxOuIuIi29717246 = -286357132;    int CarxOuIuIi29894943 = -914449968;    int CarxOuIuIi13922584 = -341892119;    int CarxOuIuIi39065137 = -2435842;    int CarxOuIuIi60903412 = -91227222;    int CarxOuIuIi245984 = 31049397;    int CarxOuIuIi46997501 = -121196807;    int CarxOuIuIi66456111 = -600159524;    int CarxOuIuIi6863310 = -969578956;    int CarxOuIuIi61902679 = 33677805;    int CarxOuIuIi34799098 = -950580064;    int CarxOuIuIi24328660 = 63821546;    int CarxOuIuIi64457728 = -268215329;    int CarxOuIuIi74728355 = -284431277;    int CarxOuIuIi89509174 = -629312017;    int CarxOuIuIi40358479 = -255649178;    int CarxOuIuIi78682819 = -876241183;    int CarxOuIuIi66401871 = -574706839;    int CarxOuIuIi94033480 = -961728260;    int CarxOuIuIi8667490 = -709715569;    int CarxOuIuIi31140371 = -728489876;    int CarxOuIuIi35792172 = -422099959;    int CarxOuIuIi39979637 = -189957948;    int CarxOuIuIi89795839 = -33598682;    int CarxOuIuIi98401417 = 59630818;    int CarxOuIuIi57551945 = 84766359;    int CarxOuIuIi63367344 = -254675155;    int CarxOuIuIi37198161 = -938752420;    int CarxOuIuIi83916116 = -724979233;     CarxOuIuIi32102387 = CarxOuIuIi9745524;     CarxOuIuIi9745524 = CarxOuIuIi44483337;     CarxOuIuIi44483337 = CarxOuIuIi10390862;     CarxOuIuIi10390862 = CarxOuIuIi76039776;     CarxOuIuIi76039776 = CarxOuIuIi47422114;     CarxOuIuIi47422114 = CarxOuIuIi7569569;     CarxOuIuIi7569569 = CarxOuIuIi10144527;     CarxOuIuIi10144527 = CarxOuIuIi13730300;     CarxOuIuIi13730300 = CarxOuIuIi89110116;     CarxOuIuIi89110116 = CarxOuIuIi17613728;     CarxOuIuIi17613728 = CarxOuIuIi68845359;     CarxOuIuIi68845359 = CarxOuIuIi40000225;     CarxOuIuIi40000225 = CarxOuIuIi18695559;     CarxOuIuIi18695559 = CarxOuIuIi75765521;     CarxOuIuIi75765521 = CarxOuIuIi53847932;     CarxOuIuIi53847932 = CarxOuIuIi18732547;     CarxOuIuIi18732547 = CarxOuIuIi78552639;     CarxOuIuIi78552639 = CarxOuIuIi70712494;     CarxOuIuIi70712494 = CarxOuIuIi84584614;     CarxOuIuIi84584614 = CarxOuIuIi70812470;     CarxOuIuIi70812470 = CarxOuIuIi32498645;     CarxOuIuIi32498645 = CarxOuIuIi12354459;     CarxOuIuIi12354459 = CarxOuIuIi30769843;     CarxOuIuIi30769843 = CarxOuIuIi57167732;     CarxOuIuIi57167732 = CarxOuIuIi67361716;     CarxOuIuIi67361716 = CarxOuIuIi80428042;     CarxOuIuIi80428042 = CarxOuIuIi60451509;     CarxOuIuIi60451509 = CarxOuIuIi57787414;     CarxOuIuIi57787414 = CarxOuIuIi44844983;     CarxOuIuIi44844983 = CarxOuIuIi60951319;     CarxOuIuIi60951319 = CarxOuIuIi37216113;     CarxOuIuIi37216113 = CarxOuIuIi43542824;     CarxOuIuIi43542824 = CarxOuIuIi59397972;     CarxOuIuIi59397972 = CarxOuIuIi93637222;     CarxOuIuIi93637222 = CarxOuIuIi6058556;     CarxOuIuIi6058556 = CarxOuIuIi44853865;     CarxOuIuIi44853865 = CarxOuIuIi89015301;     CarxOuIuIi89015301 = CarxOuIuIi48657697;     CarxOuIuIi48657697 = CarxOuIuIi56789910;     CarxOuIuIi56789910 = CarxOuIuIi45519477;     CarxOuIuIi45519477 = CarxOuIuIi9909058;     CarxOuIuIi9909058 = CarxOuIuIi32252662;     CarxOuIuIi32252662 = CarxOuIuIi65356957;     CarxOuIuIi65356957 = CarxOuIuIi64313731;     CarxOuIuIi64313731 = CarxOuIuIi50304423;     CarxOuIuIi50304423 = CarxOuIuIi5459037;     CarxOuIuIi5459037 = CarxOuIuIi45628945;     CarxOuIuIi45628945 = CarxOuIuIi36122850;     CarxOuIuIi36122850 = CarxOuIuIi93329686;     CarxOuIuIi93329686 = CarxOuIuIi70116627;     CarxOuIuIi70116627 = CarxOuIuIi71442145;     CarxOuIuIi71442145 = CarxOuIuIi96857633;     CarxOuIuIi96857633 = CarxOuIuIi64860004;     CarxOuIuIi64860004 = CarxOuIuIi92996101;     CarxOuIuIi92996101 = CarxOuIuIi99603741;     CarxOuIuIi99603741 = CarxOuIuIi97391065;     CarxOuIuIi97391065 = CarxOuIuIi13713495;     CarxOuIuIi13713495 = CarxOuIuIi53223129;     CarxOuIuIi53223129 = CarxOuIuIi8678060;     CarxOuIuIi8678060 = CarxOuIuIi66994071;     CarxOuIuIi66994071 = CarxOuIuIi47118060;     CarxOuIuIi47118060 = CarxOuIuIi52357113;     CarxOuIuIi52357113 = CarxOuIuIi68885317;     CarxOuIuIi68885317 = CarxOuIuIi28158797;     CarxOuIuIi28158797 = CarxOuIuIi80397615;     CarxOuIuIi80397615 = CarxOuIuIi25302535;     CarxOuIuIi25302535 = CarxOuIuIi80602253;     CarxOuIuIi80602253 = CarxOuIuIi25058337;     CarxOuIuIi25058337 = CarxOuIuIi69706965;     CarxOuIuIi69706965 = CarxOuIuIi8994068;     CarxOuIuIi8994068 = CarxOuIuIi29717246;     CarxOuIuIi29717246 = CarxOuIuIi29894943;     CarxOuIuIi29894943 = CarxOuIuIi13922584;     CarxOuIuIi13922584 = CarxOuIuIi39065137;     CarxOuIuIi39065137 = CarxOuIuIi60903412;     CarxOuIuIi60903412 = CarxOuIuIi245984;     CarxOuIuIi245984 = CarxOuIuIi46997501;     CarxOuIuIi46997501 = CarxOuIuIi66456111;     CarxOuIuIi66456111 = CarxOuIuIi6863310;     CarxOuIuIi6863310 = CarxOuIuIi61902679;     CarxOuIuIi61902679 = CarxOuIuIi34799098;     CarxOuIuIi34799098 = CarxOuIuIi24328660;     CarxOuIuIi24328660 = CarxOuIuIi64457728;     CarxOuIuIi64457728 = CarxOuIuIi74728355;     CarxOuIuIi74728355 = CarxOuIuIi89509174;     CarxOuIuIi89509174 = CarxOuIuIi40358479;     CarxOuIuIi40358479 = CarxOuIuIi78682819;     CarxOuIuIi78682819 = CarxOuIuIi66401871;     CarxOuIuIi66401871 = CarxOuIuIi94033480;     CarxOuIuIi94033480 = CarxOuIuIi8667490;     CarxOuIuIi8667490 = CarxOuIuIi31140371;     CarxOuIuIi31140371 = CarxOuIuIi35792172;     CarxOuIuIi35792172 = CarxOuIuIi39979637;     CarxOuIuIi39979637 = CarxOuIuIi89795839;     CarxOuIuIi89795839 = CarxOuIuIi98401417;     CarxOuIuIi98401417 = CarxOuIuIi57551945;     CarxOuIuIi57551945 = CarxOuIuIi63367344;     CarxOuIuIi63367344 = CarxOuIuIi37198161;     CarxOuIuIi37198161 = CarxOuIuIi83916116;     CarxOuIuIi83916116 = CarxOuIuIi32102387;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void URkhlORFBn82021912() {     int NUKYQJQSyN15546693 = -505644914;    int NUKYQJQSyN26617613 = -725957566;    int NUKYQJQSyN40891844 = -567248542;    int NUKYQJQSyN73183113 = -463225114;    int NUKYQJQSyN82161100 = -979175682;    int NUKYQJQSyN45926515 = -334678454;    int NUKYQJQSyN78608886 = -282892083;    int NUKYQJQSyN24491970 = -74872480;    int NUKYQJQSyN72260000 = 42284190;    int NUKYQJQSyN21655768 = -15356539;    int NUKYQJQSyN67029665 = -97180534;    int NUKYQJQSyN32050758 = -433289043;    int NUKYQJQSyN58118001 = -271830026;    int NUKYQJQSyN98322451 = -278332343;    int NUKYQJQSyN36619984 = -579493796;    int NUKYQJQSyN40237094 = -433934617;    int NUKYQJQSyN12484432 = -502661626;    int NUKYQJQSyN59404086 = -280411646;    int NUKYQJQSyN33152636 = 37051716;    int NUKYQJQSyN73952056 = -4440149;    int NUKYQJQSyN26452857 = -149098535;    int NUKYQJQSyN44645963 = -478334900;    int NUKYQJQSyN16180166 = -883684996;    int NUKYQJQSyN65476401 = -730551973;    int NUKYQJQSyN98743690 = -888218528;    int NUKYQJQSyN62703865 = -24119052;    int NUKYQJQSyN29882783 = -694908530;    int NUKYQJQSyN183135 = -80886521;    int NUKYQJQSyN93868508 = -597110951;    int NUKYQJQSyN60678324 = 34451062;    int NUKYQJQSyN79046633 = -600891056;    int NUKYQJQSyN322540 = -509530054;    int NUKYQJQSyN87081710 = -513079434;    int NUKYQJQSyN1556483 = -783662225;    int NUKYQJQSyN72363952 = -297189099;    int NUKYQJQSyN18447740 = -380519055;    int NUKYQJQSyN13468044 = -722335792;    int NUKYQJQSyN29594133 = -71429988;    int NUKYQJQSyN97652774 = -303554600;    int NUKYQJQSyN6285046 = -374974817;    int NUKYQJQSyN79168401 = -43967288;    int NUKYQJQSyN60396843 = -942250605;    int NUKYQJQSyN55819732 = -969289770;    int NUKYQJQSyN57261897 = -378781763;    int NUKYQJQSyN56258228 = -786724007;    int NUKYQJQSyN89910419 = -378597737;    int NUKYQJQSyN81474467 = -209591372;    int NUKYQJQSyN65299540 = -14965843;    int NUKYQJQSyN66969962 = -560355265;    int NUKYQJQSyN28662170 = -573410936;    int NUKYQJQSyN43374076 = -389169233;    int NUKYQJQSyN26942940 = -292081150;    int NUKYQJQSyN10970093 = -450433114;    int NUKYQJQSyN89536763 = -734379462;    int NUKYQJQSyN55569055 = -865839340;    int NUKYQJQSyN70900729 = 72689985;    int NUKYQJQSyN10437447 = -842272571;    int NUKYQJQSyN75415443 = -836696569;    int NUKYQJQSyN74439422 = -575006586;    int NUKYQJQSyN19457235 = -855056630;    int NUKYQJQSyN16043732 = -639769924;    int NUKYQJQSyN78425752 = -102005563;    int NUKYQJQSyN30623461 = -477761530;    int NUKYQJQSyN11581677 = -992166872;    int NUKYQJQSyN42609135 = -414465483;    int NUKYQJQSyN66707125 = -587650480;    int NUKYQJQSyN44969047 = -920209610;    int NUKYQJQSyN56561518 = -488167801;    int NUKYQJQSyN25958499 = -981143245;    int NUKYQJQSyN18172245 = -98974741;    int NUKYQJQSyN26769050 = -711598825;    int NUKYQJQSyN82890298 = -331231639;    int NUKYQJQSyN61751311 = -976857046;    int NUKYQJQSyN26867591 = -587973467;    int NUKYQJQSyN94783654 = -960472861;    int NUKYQJQSyN66056013 = -206847931;    int NUKYQJQSyN88826231 = -509045130;    int NUKYQJQSyN58918269 = -404903234;    int NUKYQJQSyN9218173 = -943827966;    int NUKYQJQSyN8833272 = -409620791;    int NUKYQJQSyN81229398 = -814527681;    int NUKYQJQSyN64583242 = -579942688;    int NUKYQJQSyN33213173 = -520531256;    int NUKYQJQSyN65206338 = 76299984;    int NUKYQJQSyN17304248 = -576379706;    int NUKYQJQSyN52103694 = -208809906;    int NUKYQJQSyN89352447 = 40903059;    int NUKYQJQSyN97544947 = -778699973;    int NUKYQJQSyN45987427 = -917822886;    int NUKYQJQSyN1463224 = -269879084;    int NUKYQJQSyN8010293 = -538246485;    int NUKYQJQSyN38052600 = -885639224;    int NUKYQJQSyN55154710 = -496423402;    int NUKYQJQSyN78195540 = -448497971;    int NUKYQJQSyN90241313 = -735204894;    int NUKYQJQSyN742650 = -941961726;    int NUKYQJQSyN29773382 = -364489076;    int NUKYQJQSyN44238055 = -977122898;    int NUKYQJQSyN14652763 = -964316280;    int NUKYQJQSyN89551102 = -505644914;     NUKYQJQSyN15546693 = NUKYQJQSyN26617613;     NUKYQJQSyN26617613 = NUKYQJQSyN40891844;     NUKYQJQSyN40891844 = NUKYQJQSyN73183113;     NUKYQJQSyN73183113 = NUKYQJQSyN82161100;     NUKYQJQSyN82161100 = NUKYQJQSyN45926515;     NUKYQJQSyN45926515 = NUKYQJQSyN78608886;     NUKYQJQSyN78608886 = NUKYQJQSyN24491970;     NUKYQJQSyN24491970 = NUKYQJQSyN72260000;     NUKYQJQSyN72260000 = NUKYQJQSyN21655768;     NUKYQJQSyN21655768 = NUKYQJQSyN67029665;     NUKYQJQSyN67029665 = NUKYQJQSyN32050758;     NUKYQJQSyN32050758 = NUKYQJQSyN58118001;     NUKYQJQSyN58118001 = NUKYQJQSyN98322451;     NUKYQJQSyN98322451 = NUKYQJQSyN36619984;     NUKYQJQSyN36619984 = NUKYQJQSyN40237094;     NUKYQJQSyN40237094 = NUKYQJQSyN12484432;     NUKYQJQSyN12484432 = NUKYQJQSyN59404086;     NUKYQJQSyN59404086 = NUKYQJQSyN33152636;     NUKYQJQSyN33152636 = NUKYQJQSyN73952056;     NUKYQJQSyN73952056 = NUKYQJQSyN26452857;     NUKYQJQSyN26452857 = NUKYQJQSyN44645963;     NUKYQJQSyN44645963 = NUKYQJQSyN16180166;     NUKYQJQSyN16180166 = NUKYQJQSyN65476401;     NUKYQJQSyN65476401 = NUKYQJQSyN98743690;     NUKYQJQSyN98743690 = NUKYQJQSyN62703865;     NUKYQJQSyN62703865 = NUKYQJQSyN29882783;     NUKYQJQSyN29882783 = NUKYQJQSyN183135;     NUKYQJQSyN183135 = NUKYQJQSyN93868508;     NUKYQJQSyN93868508 = NUKYQJQSyN60678324;     NUKYQJQSyN60678324 = NUKYQJQSyN79046633;     NUKYQJQSyN79046633 = NUKYQJQSyN322540;     NUKYQJQSyN322540 = NUKYQJQSyN87081710;     NUKYQJQSyN87081710 = NUKYQJQSyN1556483;     NUKYQJQSyN1556483 = NUKYQJQSyN72363952;     NUKYQJQSyN72363952 = NUKYQJQSyN18447740;     NUKYQJQSyN18447740 = NUKYQJQSyN13468044;     NUKYQJQSyN13468044 = NUKYQJQSyN29594133;     NUKYQJQSyN29594133 = NUKYQJQSyN97652774;     NUKYQJQSyN97652774 = NUKYQJQSyN6285046;     NUKYQJQSyN6285046 = NUKYQJQSyN79168401;     NUKYQJQSyN79168401 = NUKYQJQSyN60396843;     NUKYQJQSyN60396843 = NUKYQJQSyN55819732;     NUKYQJQSyN55819732 = NUKYQJQSyN57261897;     NUKYQJQSyN57261897 = NUKYQJQSyN56258228;     NUKYQJQSyN56258228 = NUKYQJQSyN89910419;     NUKYQJQSyN89910419 = NUKYQJQSyN81474467;     NUKYQJQSyN81474467 = NUKYQJQSyN65299540;     NUKYQJQSyN65299540 = NUKYQJQSyN66969962;     NUKYQJQSyN66969962 = NUKYQJQSyN28662170;     NUKYQJQSyN28662170 = NUKYQJQSyN43374076;     NUKYQJQSyN43374076 = NUKYQJQSyN26942940;     NUKYQJQSyN26942940 = NUKYQJQSyN10970093;     NUKYQJQSyN10970093 = NUKYQJQSyN89536763;     NUKYQJQSyN89536763 = NUKYQJQSyN55569055;     NUKYQJQSyN55569055 = NUKYQJQSyN70900729;     NUKYQJQSyN70900729 = NUKYQJQSyN10437447;     NUKYQJQSyN10437447 = NUKYQJQSyN75415443;     NUKYQJQSyN75415443 = NUKYQJQSyN74439422;     NUKYQJQSyN74439422 = NUKYQJQSyN19457235;     NUKYQJQSyN19457235 = NUKYQJQSyN16043732;     NUKYQJQSyN16043732 = NUKYQJQSyN78425752;     NUKYQJQSyN78425752 = NUKYQJQSyN30623461;     NUKYQJQSyN30623461 = NUKYQJQSyN11581677;     NUKYQJQSyN11581677 = NUKYQJQSyN42609135;     NUKYQJQSyN42609135 = NUKYQJQSyN66707125;     NUKYQJQSyN66707125 = NUKYQJQSyN44969047;     NUKYQJQSyN44969047 = NUKYQJQSyN56561518;     NUKYQJQSyN56561518 = NUKYQJQSyN25958499;     NUKYQJQSyN25958499 = NUKYQJQSyN18172245;     NUKYQJQSyN18172245 = NUKYQJQSyN26769050;     NUKYQJQSyN26769050 = NUKYQJQSyN82890298;     NUKYQJQSyN82890298 = NUKYQJQSyN61751311;     NUKYQJQSyN61751311 = NUKYQJQSyN26867591;     NUKYQJQSyN26867591 = NUKYQJQSyN94783654;     NUKYQJQSyN94783654 = NUKYQJQSyN66056013;     NUKYQJQSyN66056013 = NUKYQJQSyN88826231;     NUKYQJQSyN88826231 = NUKYQJQSyN58918269;     NUKYQJQSyN58918269 = NUKYQJQSyN9218173;     NUKYQJQSyN9218173 = NUKYQJQSyN8833272;     NUKYQJQSyN8833272 = NUKYQJQSyN81229398;     NUKYQJQSyN81229398 = NUKYQJQSyN64583242;     NUKYQJQSyN64583242 = NUKYQJQSyN33213173;     NUKYQJQSyN33213173 = NUKYQJQSyN65206338;     NUKYQJQSyN65206338 = NUKYQJQSyN17304248;     NUKYQJQSyN17304248 = NUKYQJQSyN52103694;     NUKYQJQSyN52103694 = NUKYQJQSyN89352447;     NUKYQJQSyN89352447 = NUKYQJQSyN97544947;     NUKYQJQSyN97544947 = NUKYQJQSyN45987427;     NUKYQJQSyN45987427 = NUKYQJQSyN1463224;     NUKYQJQSyN1463224 = NUKYQJQSyN8010293;     NUKYQJQSyN8010293 = NUKYQJQSyN38052600;     NUKYQJQSyN38052600 = NUKYQJQSyN55154710;     NUKYQJQSyN55154710 = NUKYQJQSyN78195540;     NUKYQJQSyN78195540 = NUKYQJQSyN90241313;     NUKYQJQSyN90241313 = NUKYQJQSyN742650;     NUKYQJQSyN742650 = NUKYQJQSyN29773382;     NUKYQJQSyN29773382 = NUKYQJQSyN44238055;     NUKYQJQSyN44238055 = NUKYQJQSyN14652763;     NUKYQJQSyN14652763 = NUKYQJQSyN89551102;     NUKYQJQSyN89551102 = NUKYQJQSyN15546693;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VaybhdIrHV52477785() {     int EcxgpoxWeB28332860 = -640685606;    int EcxgpoxWeB13388501 = -896291580;    int EcxgpoxWeB58644644 = -975607924;    int EcxgpoxWeB81123938 = -287427391;    int EcxgpoxWeB75672204 = -370992147;    int EcxgpoxWeB89236585 = -111601767;    int EcxgpoxWeB99816644 = -828564153;    int EcxgpoxWeB51211082 = -519390737;    int EcxgpoxWeB67664811 = 80526098;    int EcxgpoxWeB27874473 = -919444172;    int EcxgpoxWeB94517047 = -211866549;    int EcxgpoxWeB64001263 = -940621859;    int EcxgpoxWeB28261304 = -175612683;    int EcxgpoxWeB47946530 = -610747541;    int EcxgpoxWeB59926537 = -486479626;    int EcxgpoxWeB68662162 = -984826006;    int EcxgpoxWeB68236249 = -166436093;    int EcxgpoxWeB42962571 = -935621949;    int EcxgpoxWeB34313267 = -972417480;    int EcxgpoxWeB76606552 = -603684288;    int EcxgpoxWeB6626914 = -640414800;    int EcxgpoxWeB73693782 = -648193248;    int EcxgpoxWeB54940462 = -814688891;    int EcxgpoxWeB9983531 = 9103014;    int EcxgpoxWeB4637950 = 1339547;    int EcxgpoxWeB44022511 = -814550251;    int EcxgpoxWeB41298319 = -228614810;    int EcxgpoxWeB67536059 = -254684625;    int EcxgpoxWeB10485562 = -811328191;    int EcxgpoxWeB13429440 = -695435093;    int EcxgpoxWeB23482630 = -877307806;    int EcxgpoxWeB3641364 = -488249733;    int EcxgpoxWeB33127660 = -388934219;    int EcxgpoxWeB78632855 = -688416444;    int EcxgpoxWeB55424484 = -907220979;    int EcxgpoxWeB13671855 = -169082159;    int EcxgpoxWeB50232104 = -512245143;    int EcxgpoxWeB87298155 = 8949921;    int EcxgpoxWeB3428161 = -145405178;    int EcxgpoxWeB32549775 = -219391663;    int EcxgpoxWeB40599710 = -419441281;    int EcxgpoxWeB68099299 = -840561939;    int EcxgpoxWeB90703201 = -992723215;    int EcxgpoxWeB1890436 = -719954006;    int EcxgpoxWeB36266718 = -679493968;    int EcxgpoxWeB63538548 = -451916177;    int EcxgpoxWeB59056487 = -707106424;    int EcxgpoxWeB94739506 = -170318622;    int EcxgpoxWeB11034012 = -723957240;    int EcxgpoxWeB75612590 = -84447399;    int EcxgpoxWeB61701569 = -642053541;    int EcxgpoxWeB55119487 = -763266488;    int EcxgpoxWeB45455243 = -626436396;    int EcxgpoxWeB34236489 = -323831204;    int EcxgpoxWeB45850872 = -860283826;    int EcxgpoxWeB54639077 = -992492359;    int EcxgpoxWeB58448039 = 18397310;    int EcxgpoxWeB48661114 = -884710938;    int EcxgpoxWeB76485988 = -188766939;    int EcxgpoxWeB31649694 = -556441896;    int EcxgpoxWeB47938266 = -882986958;    int EcxgpoxWeB32280586 = -473879529;    int EcxgpoxWeB40725521 = -708062546;    int EcxgpoxWeB54235372 = -224038810;    int EcxgpoxWeB4391844 = 57863634;    int EcxgpoxWeB90875684 = -723616817;    int EcxgpoxWeB30873604 = -451687641;    int EcxgpoxWeB49628449 = -487196240;    int EcxgpoxWeB92522045 = -703526562;    int EcxgpoxWeB46254683 = -217397468;    int EcxgpoxWeB18430059 = -372580864;    int EcxgpoxWeB80938093 = -75386014;    int EcxgpoxWeB39534410 = -690216771;    int EcxgpoxWeB1763492 = -653025817;    int EcxgpoxWeB36006843 = -84243007;    int EcxgpoxWeB38527615 = -799852861;    int EcxgpoxWeB82990581 = -655470033;    int EcxgpoxWeB53050026 = 5265114;    int EcxgpoxWeB73716812 = -311403019;    int EcxgpoxWeB41099402 = -546744276;    int EcxgpoxWeB84966023 = -7443828;    int EcxgpoxWeB46558812 = 41703811;    int EcxgpoxWeB56502048 = -530727386;    int EcxgpoxWeB34872971 = -626880793;    int EcxgpoxWeB51727870 = 46618448;    int EcxgpoxWeB68363142 = -14041318;    int EcxgpoxWeB58186120 = -861813337;    int EcxgpoxWeB98891171 = 34896985;    int EcxgpoxWeB32781984 = -828132618;    int EcxgpoxWeB785408 = -914728621;    int EcxgpoxWeB55223815 = -87479470;    int EcxgpoxWeB1570991 = -627534205;    int EcxgpoxWeB10812167 = -802283141;    int EcxgpoxWeB71778467 = -588963282;    int EcxgpoxWeB84611508 = -336404706;    int EcxgpoxWeB8319125 = -945561753;    int EcxgpoxWeB27373779 = -32499393;    int EcxgpoxWeB36467830 = -668684405;    int EcxgpoxWeB97498591 = -677817640;    int EcxgpoxWeB45391034 = -640685606;     EcxgpoxWeB28332860 = EcxgpoxWeB13388501;     EcxgpoxWeB13388501 = EcxgpoxWeB58644644;     EcxgpoxWeB58644644 = EcxgpoxWeB81123938;     EcxgpoxWeB81123938 = EcxgpoxWeB75672204;     EcxgpoxWeB75672204 = EcxgpoxWeB89236585;     EcxgpoxWeB89236585 = EcxgpoxWeB99816644;     EcxgpoxWeB99816644 = EcxgpoxWeB51211082;     EcxgpoxWeB51211082 = EcxgpoxWeB67664811;     EcxgpoxWeB67664811 = EcxgpoxWeB27874473;     EcxgpoxWeB27874473 = EcxgpoxWeB94517047;     EcxgpoxWeB94517047 = EcxgpoxWeB64001263;     EcxgpoxWeB64001263 = EcxgpoxWeB28261304;     EcxgpoxWeB28261304 = EcxgpoxWeB47946530;     EcxgpoxWeB47946530 = EcxgpoxWeB59926537;     EcxgpoxWeB59926537 = EcxgpoxWeB68662162;     EcxgpoxWeB68662162 = EcxgpoxWeB68236249;     EcxgpoxWeB68236249 = EcxgpoxWeB42962571;     EcxgpoxWeB42962571 = EcxgpoxWeB34313267;     EcxgpoxWeB34313267 = EcxgpoxWeB76606552;     EcxgpoxWeB76606552 = EcxgpoxWeB6626914;     EcxgpoxWeB6626914 = EcxgpoxWeB73693782;     EcxgpoxWeB73693782 = EcxgpoxWeB54940462;     EcxgpoxWeB54940462 = EcxgpoxWeB9983531;     EcxgpoxWeB9983531 = EcxgpoxWeB4637950;     EcxgpoxWeB4637950 = EcxgpoxWeB44022511;     EcxgpoxWeB44022511 = EcxgpoxWeB41298319;     EcxgpoxWeB41298319 = EcxgpoxWeB67536059;     EcxgpoxWeB67536059 = EcxgpoxWeB10485562;     EcxgpoxWeB10485562 = EcxgpoxWeB13429440;     EcxgpoxWeB13429440 = EcxgpoxWeB23482630;     EcxgpoxWeB23482630 = EcxgpoxWeB3641364;     EcxgpoxWeB3641364 = EcxgpoxWeB33127660;     EcxgpoxWeB33127660 = EcxgpoxWeB78632855;     EcxgpoxWeB78632855 = EcxgpoxWeB55424484;     EcxgpoxWeB55424484 = EcxgpoxWeB13671855;     EcxgpoxWeB13671855 = EcxgpoxWeB50232104;     EcxgpoxWeB50232104 = EcxgpoxWeB87298155;     EcxgpoxWeB87298155 = EcxgpoxWeB3428161;     EcxgpoxWeB3428161 = EcxgpoxWeB32549775;     EcxgpoxWeB32549775 = EcxgpoxWeB40599710;     EcxgpoxWeB40599710 = EcxgpoxWeB68099299;     EcxgpoxWeB68099299 = EcxgpoxWeB90703201;     EcxgpoxWeB90703201 = EcxgpoxWeB1890436;     EcxgpoxWeB1890436 = EcxgpoxWeB36266718;     EcxgpoxWeB36266718 = EcxgpoxWeB63538548;     EcxgpoxWeB63538548 = EcxgpoxWeB59056487;     EcxgpoxWeB59056487 = EcxgpoxWeB94739506;     EcxgpoxWeB94739506 = EcxgpoxWeB11034012;     EcxgpoxWeB11034012 = EcxgpoxWeB75612590;     EcxgpoxWeB75612590 = EcxgpoxWeB61701569;     EcxgpoxWeB61701569 = EcxgpoxWeB55119487;     EcxgpoxWeB55119487 = EcxgpoxWeB45455243;     EcxgpoxWeB45455243 = EcxgpoxWeB34236489;     EcxgpoxWeB34236489 = EcxgpoxWeB45850872;     EcxgpoxWeB45850872 = EcxgpoxWeB54639077;     EcxgpoxWeB54639077 = EcxgpoxWeB58448039;     EcxgpoxWeB58448039 = EcxgpoxWeB48661114;     EcxgpoxWeB48661114 = EcxgpoxWeB76485988;     EcxgpoxWeB76485988 = EcxgpoxWeB31649694;     EcxgpoxWeB31649694 = EcxgpoxWeB47938266;     EcxgpoxWeB47938266 = EcxgpoxWeB32280586;     EcxgpoxWeB32280586 = EcxgpoxWeB40725521;     EcxgpoxWeB40725521 = EcxgpoxWeB54235372;     EcxgpoxWeB54235372 = EcxgpoxWeB4391844;     EcxgpoxWeB4391844 = EcxgpoxWeB90875684;     EcxgpoxWeB90875684 = EcxgpoxWeB30873604;     EcxgpoxWeB30873604 = EcxgpoxWeB49628449;     EcxgpoxWeB49628449 = EcxgpoxWeB92522045;     EcxgpoxWeB92522045 = EcxgpoxWeB46254683;     EcxgpoxWeB46254683 = EcxgpoxWeB18430059;     EcxgpoxWeB18430059 = EcxgpoxWeB80938093;     EcxgpoxWeB80938093 = EcxgpoxWeB39534410;     EcxgpoxWeB39534410 = EcxgpoxWeB1763492;     EcxgpoxWeB1763492 = EcxgpoxWeB36006843;     EcxgpoxWeB36006843 = EcxgpoxWeB38527615;     EcxgpoxWeB38527615 = EcxgpoxWeB82990581;     EcxgpoxWeB82990581 = EcxgpoxWeB53050026;     EcxgpoxWeB53050026 = EcxgpoxWeB73716812;     EcxgpoxWeB73716812 = EcxgpoxWeB41099402;     EcxgpoxWeB41099402 = EcxgpoxWeB84966023;     EcxgpoxWeB84966023 = EcxgpoxWeB46558812;     EcxgpoxWeB46558812 = EcxgpoxWeB56502048;     EcxgpoxWeB56502048 = EcxgpoxWeB34872971;     EcxgpoxWeB34872971 = EcxgpoxWeB51727870;     EcxgpoxWeB51727870 = EcxgpoxWeB68363142;     EcxgpoxWeB68363142 = EcxgpoxWeB58186120;     EcxgpoxWeB58186120 = EcxgpoxWeB98891171;     EcxgpoxWeB98891171 = EcxgpoxWeB32781984;     EcxgpoxWeB32781984 = EcxgpoxWeB785408;     EcxgpoxWeB785408 = EcxgpoxWeB55223815;     EcxgpoxWeB55223815 = EcxgpoxWeB1570991;     EcxgpoxWeB1570991 = EcxgpoxWeB10812167;     EcxgpoxWeB10812167 = EcxgpoxWeB71778467;     EcxgpoxWeB71778467 = EcxgpoxWeB84611508;     EcxgpoxWeB84611508 = EcxgpoxWeB8319125;     EcxgpoxWeB8319125 = EcxgpoxWeB27373779;     EcxgpoxWeB27373779 = EcxgpoxWeB36467830;     EcxgpoxWeB36467830 = EcxgpoxWeB97498591;     EcxgpoxWeB97498591 = EcxgpoxWeB45391034;     EcxgpoxWeB45391034 = EcxgpoxWeB28332860;}
// Junk Finished
