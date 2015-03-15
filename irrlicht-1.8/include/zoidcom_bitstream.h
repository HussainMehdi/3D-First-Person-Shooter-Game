/****************************************
* zoidcom_bitstream.h
* bitstream class
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

/** \file zoidcom_bitstream.h
*/

#ifndef _ZCOM_BITSTREAM_H_
#define _ZCOM_BITSTREAM_H_

#include "zoidcom.h"

/** @brief Bitstreams are Zoidcom's native data exchange objects.
*/
class ZCOM_API ZCom_BitStream
{
public:
  /// @brief Position in bitstream.
  struct BitPos
  {
    /// bit in int
    zU16 bit;
    /// pos in array
    zU16 pos;
  };
protected:
  // array holding the data
  zU8*   m_ar;
  // fill counter in bits
  zU32    m_fill;
  BitPos  m_fillpos, m_readpos;
  // size of array in bytes
  zU16    m_size;
  // max fill in bytes
  zU16    m_maxfill;

  zU8     m_flags;
public:
  /// @brief Constructor.
  /// @param _maxfill Expected amount of bytes the stream will hold.
  ZCom_BitStream( zU16 _maxfill = 64 );
  /// @brief Copy constructor.
  ZCom_BitStream( const ZCom_BitStream &_str );
  /// @brief Destructor.
  ~ZCom_BitStream();

  /// @brief Remove all data and reset position counters.
  void Clear();

  /// @brief Make exact copy of this object, including position counters.
  /// @note Some bitstreams received in Zoidcom callbacks cannot be duplicated.
  ///       In this cases NULL is returned.
  ZCom_BitStream *Duplicate() const;

  /** 
   * @name Stream position operations.
   * @{
   */

  /// @brief Store write state.
  /// @param _pos Store target.
  void saveWriteState( BitPos &_pos ) const {_pos = m_fillpos;}

  /// @brief Restore write state. (Undos all add*** calls since 'saveWriteState')
  /// @param _pos Previously stored BitPos object.
  void restoreWriteState( const BitPos &_pos ) {m_fillpos = _pos; m_fill = m_fillpos.pos * 8 + m_fillpos.bit; }

  /// @brief Store read state.
  /// @param _pos Store target.
  void saveReadState( BitPos &_pos ) const {_pos = m_readpos;}

  /// @brief Restore read state (Undos all get*** calls since 'saveReadState')
  /// @param _pos Previously stored BitPos object.
  void restoreReadState( const BitPos &_pos ) {m_readpos = _pos;}

  /// @brief Reset read state to beginning of stream.
  void resetReadState() {m_readpos.bit = 0;m_readpos.pos = 0;}

  /// @brief Log current read position to the log output.
  void logReadState();

  /// @brief Log current write position to the log output.
  void logWriteState();

  /// @}

  /**
   * @name Data size checks.
   * @{
   */

  /// @brief Check if stream has enough space for _bits more bits.
  /// @param _bits The amount of bits to check against.
  /// @returns 'true' if enough space is left.
  bool checkMax( zU32 _bits );

  /// @brief Check if stream limit has been exceeded.
  /// @returns 'true' if self imposed max fill has been exceeded.
  inline bool checkFull() const {return ( m_fill / 8 > m_maxfill );}

  /// @brief Check if stream read pos is at end of stream.
  /// @returns 'true' if end of stream has been reached
  inline bool endOfStream() const {return m_readpos.pos * 8 + m_readpos.bit > m_fillpos.pos * 8 + m_fillpos.bit;}

  /// @brief Get amount of bits stored in the stream.
  /// @returns # of bits in the stream.
  inline zU32 getBitCount() const {return m_fillpos.pos * 8 + m_fillpos.bit;}
  // @}

  /**
   * @name (De-)Serialization.
   * @{
   */

  /// @brief Serialize stream to array.
  /// @param _ptr Store target.
  /// @param _size Pointer to an int where the stored amount of bytes will be written.
  /// @param _max_size Size of the storage array.
  /// @returns 'true' on success.
  bool Serialize( char *_ptr, zU16 *_size, zU16 _max_size );

  /// @brief Deserialize array to stream.
  /// @param _ptr Pointer to the serialized array.
  /// @param _size Size of that array.
  /// @returns 'true' if no errors occured.
  bool Deserialize( char *_ptr, zU16 _size );

  /// @brief Get needed size for serialize buffer.
  /// @returns Amount of bytes needed to serialize the current stream.
  zU16 getSizeHint( void );

  //@}

  /**
   * @name Data operations
   * @{
   */

  /// @brief Add an int to the stream.
  /// @note The sign bit is only added if enough bits, in most cases 32, are used.
  ///       For adding signed data use addSignedInt() instead.
  /// @param _data Input data.
  /// @param _bits Amount of bits to store.
  /// @returns 'true' on success.
  bool addInt( zU32 _data, zU8 _bits );

  /// @brief Retrieve an int.
  /// @param _bits Amount of bits to retrieve.
  /// @returns The int.
  zU32 getInt( zU8 _bits );

  /// @brief Skip an int.
  /// @param _bits Amount of bits used to store the int.
  void skipInt( zU8 _bits );

  /// @brief Add a signed int to the stream.
  /// @param _data Input data.
  /// @param _bits Amount of bits to store.
  /// @note One extra bit is stored for sign.
  /// @returns 'true' on success.
  bool addSignedInt( zS32 _data, zU8 _bits );

  /// @brief Retrieve signed int.
  /// @param _bits Amount of bits to retrieve.
  /// @returns The int.
  zS32 getSignedInt( zU8 _bits );

  /// @brief Skip a signed int.
  /// @param _bits Amount of bits used to store the signed int.
  void skipSignedInt( zU8 _bits );

  /// @brief Add a bool to the stream.
  /// @param _b Input data.
  /// @returns 'true' on success.
  bool addBool( bool _b );

  /// @brief Retrieve a bool.
  /// @returns The bool.
  bool getBool();

  /// @brief Skip a bool
  void skipBool();

  /// @brief Add a float to the stream.
  /// @param _f Input data.
  /// @param _mant_bits Amount of mantissa bits to store. For more details look \ref ZCom_Node::addReplicationFloat().
  /// @returns 'true' on success.
  bool addFloat( zFloat _f, zU8 _mant_bits );

  /// @brief Retrieve a float.
  /// @param _mant_bits Amount of mantissa bits that have been stored with addFloat().
  /// @returns The float.
  zFloat getFloat( zU8 _mant_bits );

  /// @brief Skip a float.
  /// @param _mant_bits Mantissa bits used to store the float.
  void skipFloat( zU8 _mant_bits );

  /// @brief Add a string to the stream.
  /// @param _string NULL-terminated c-string.
  /// @returns 'true' on success.
  bool addString( const char *_string );

  /// @brief Get size of upcoming string.
  /// @returns If a string or widestring is the next data in the stream call this to get its size (in bytes), without trailing 0.
  zU16 getStringSize();

  /// @brief Get length of upcoming string.
  /// @returns If a string is the next data in the stream call this to get its length (number of characters), without trailing 0.
  zU16 getStringLength();

  /// @brief Retrieve a string.
  /// @param _buf Should point to an allocated buffer.
  /// @param _maxsize Size of that buffer. Find size of incoming string with getStringSize().
  void getString( char *_buf, zU16 _maxsize );

  /// @brief Retrieve a string via static buffer.
  /// @returns Pointer to a static buffer containing the next string in the stream.
  ///
  /// The buffer is of size 2048, longer strings are cut. The next call to this method will overwrite the content of this buffer.
  /// @note This method is thread-safe.
  const char* getStringStatic();

  /// @brief Skip a string or wstring.
  void skipString();

  /// @brief Add a string to the stream.
  /// @param _string NULL-terminated c-string.
  /// @returns 'true' on success.
  bool addStringW( const wchar_t *_string );

  /// @brief Get size of upcoming string.
  /// @returns If a string is the next data in the stream call this to get its length, without trailing 0.

  /// @brief Retrieve a string.
  /// @param _buf Should point to an allocated buffer.
  /// @param _maxsize Size of that buffer (elementcount, not bytes). Find size of incoming string with getStringWLength().
  void getStringW( wchar_t *_buf, zU16 _maxsize );

  /// @brief Get length of upcoming string.
  /// @returns If a string is the next data in the stream call this to get its length (number of characters), without trailing 0.
  zU16 getStringWLength();

  /// @brief Retrieve a string via static buffer.
  /// @returns Pointer to a static buffer containing the next string in the stream.
  ///
  /// The buffer can hold wide strings of upto 1024 wchars, longer strings are cut. The next call to this method will overwrite 
  /// the content of this buffer.
  /// @note This method is thread-safe.
  const wchar_t* getStringWStatic();

  /// @brief Add a buffer to the stream.
  /// @param _buf Pointer to the buffer.
  /// @param _size of the buffer.
  /// @returns 'true' on success.
  bool addBuffer( char *_buf, zU16 _size );

  /// @brief Retrieve a buffer.
  /// @param _buf Pointer to allocated array.
  /// @param _bytes Size of that array.
  /// @returns The amount of bytes really read (is smaller for end of stream conditions).
  zU16 getBuffer( char *_buf, zU16 _bytes );

  /// @brief Skip a stored buffer in the stream.
  /// @param _bytes The size of the buffer to skip.
  void skipBuffer(zU16 _bytes);

  /// @brief Get amount of bytes available for buffer get.
  /// @returns Maximum size a getBuffer() would be able to retrieve.
  zU16  getBufferMax( void );

  /// @brief Add an existing bitstream to the stream.
  /// @param _stream The bitstream to attach.
  /// @param _allow_align Use allow_align=true only if you intent to get the stream back as a whole with getBitStream(.., true)
  ///                     Otherwise you cannot know how many padding bits have been added.
  /// @returns 'true' on success.
  ///
  /// For extracting the stream again with getBitStream() the same value for _allow_align must be used.
  bool addBitStream( ZCom_BitStream *_stream, bool _allow_align = false );

  /// @brief Retrieve a whole bitstream.
  /// @param _bits Amount of bits to retrieve.
  /// @param _allow_align See addBitStream().
  ///
  /// The application is responsible for deleting the resulting object.
  ZCom_BitStream *getBitStream( zU32 _bits, bool _allow_align = false );

  /// @brief Skip some bits in the stream. This only affects the get* methods.
  /// @param _amount Number of bits to skip.
  void skipBits(zU32 _amount);
  // @}

  /// @brief Check if two bitstreams contain the same data
  /// @returns true If two bitstreams have the same contents.
  bool isEqual(const ZCom_BitStream& _other) const;

  /// @brief Overloaded memory operator ensuring that always Zoidcom's new gets called.
  void* operator new(size_t _size);
  /// @brief Overloaded memory operator ensuring that always Zoidcom's delete gets called.
  void  operator delete(void *_p);

  /// @brief Assignment operator.
  ZCom_BitStream& operator=(const ZCom_BitStream &_str);

  // only for zoidcom internal use
  char *getString();
protected:
  bool checkSize( zU32 _bits );
  inline void incPos( struct BitPos *_pos );
};

#endif


