/****************************************
* zoidcom_replicator_movement.h
* extrapolating replicator
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/


#ifndef _ZOIDREPLICATORMOVEMENT_H_
#define _ZOIDREPLICATORMOVEMENT_H_

/** @file zoidcom_replicator_movement.h
*/

#include "zoidcom.h"
#include "zoidcom_typehelper.h"

// forward
template <typename T, int SIZE> class ZCom_ReplicateMovementPrivate;

/** 
 * @name Extrapolator flags
 * Stored in m_extflags in ZCom_RSetupExtrapolate.
 */

///@{
/// If set, use error function, else use constant error
#define ZCOM_REPMOVE_USE_ERROR_FUNC                   (1L << 2)
///@}

//////////////////////////////////////////////////////////////////////////
// Replicator parameter class
//////////////////////////////////////////////////////////////////////////

/** @brief Parameter class for movement replicators.
*/
template <typename T>
class ZCOM_API ZCom_RSetupMovement : public ZCom_RSetupNumeric
{
public:
  /// constructor
  ZCOM_TAPI ZCom_RSetupMovement(zU8 _rbits, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0,
                                  zS16 _mindelay = -1, zS16 _maxdelay = -1)
  : ZCom_RSetupNumeric(_rbits, _flags, _rules, _intercept_id, _mindelay, _maxdelay)
  {
    m_inputsizebits = 8;
    m_extflags = 0;
	  m_interpolation_time = 100;
    m_error.constant = 25;
  }

  ZCOM_TAPI ZCom_ReplicatorSetup* Duplicate();

  /**
   * @brief Set amount of bits needed to describe the largest size of the input bitstream in bits.
   * 
   * Ex.: If the input bitstream has a maximum size of 255 bits, set this value to 8.
   *
   * @attention Don't change value during operation of replicator.
   */
  void setInputsizeBits(zU8 _inputbits) { m_inputsizebits = _inputbits; }
  /**
   * @brief Get input size bits.
   */
  zU8 getInputsizeBits() const { return m_inputsizebits; }

  /// Get extended flags.
  zU8 getExtendedFlags() const { return m_extflags; }

  /// set constant error threshold
  void setConstantErrorThreshold(zFloat _threshold) { m_error.constant = _threshold; }
  /// get constant error threshold
  zFloat getConstantErrorThreshold() const { return m_error.constant; }

  /// get interpolation time in msecs
  zU32 getInterpolationTime() const { return m_interpolation_time; }

  /// set interpolation time
  void setInterpolationTime(zU32 _time) { m_interpolation_time = _time; }

protected:
  zU8   m_extflags;
  zU8   m_inputsizebits;
  zU32  m_interpolation_time;

  // constant error threshold or error function
  union {
    zFloat constant;
  } m_error;
};

//////////////////////////////////////////////////////////////////////////
// Movement replicator callbacks
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Callback interface for movement update related information.
 */
template <typename T>
class ZCOM_API ZCom_MoveUpdateListener
{
public:
  /**
   * @brief New input data arrived from owner. (Appears on: Auth)
   * @param _inputstream The inputstream provided to ZCom_Replicate_Movement::updateInput()
   */
  virtual void 
    inputUpdated(ZCom_BitStream& _inputstream, bool _inputchanged, zU32 _client_time, zU32 _estimated_time_sent) = 0;

  /**
   * @brief The input bitstream has just been sent to the authority (Appears on: Owner)
   * @param _inputstream The inputstream provided to ZCom_Replicate_Movement::updateInput()
   */
  virtual void 
    inputSent(ZCom_BitStream& _inputstream) = 0;

  /**
   * @brief Correction received by authority. (Appears on: Owner)
   * @param _pos Position vector.
   * @param _vel Velocity vector.
   * @param _acc Acceleration vector.
   * @param _teleport If true, object was teleported.
   * @param _estimated_time_sent Estimated time of event leaving remote authority.
   */
  virtual void 
    correctionReceived(T *_pos, zFloat* _vel, zFloat *_acc, bool _teleport, zU32 _estimated_time_sent) = 0;

  /**
   * @brief Update received by authority. (Appears on: Proxy)
   */
  virtual void 
    updateReceived(ZCom_BitStream& _inputstream, T *_pos, zFloat* _vel, zFloat *_acc, zU32 _estimated_time_sent) = 0;
};


//////////////////////////////////////////////////////////////////////////
// Movement replicator implementation
//////////////////////////////////////////////////////////////////////////

/** 
 * @brief Replicator for object positions.
 * @param T Type of position vector. Supported are zU32, zS32 and zFloat.
 * @param SIZE Size of position, velocity and accel vectors.
 *
 * This replicator replicates position, velocity and acceleration vectors for an object.
 * The dimension of the vectors is given by the SIZE parameter to the template.
 * ZCom_Replicate_Movement<zFloat, 3> will replicate 3D vectors of floats.
 *
 * Peeking is not supported for this replicator.
 *
 * See ex07_movement (in the samples directory) for example usage.
 * @see @ref playermovement
 */
template <typename T, int SIZE>
class ZCOM_API ZCom_Replicate_Movement : public ZCom_ReplicatorAdvanced
{
  // make T a non-pointer
  typedef typename ZCom_TypeHelper<T>::value_type TYPE;
public:
  // forward
  class UpdateListener;
public:
  /// constructor, taking a ZCom_ReplicatorSetup pointer
  ZCOM_TAPI ZCom_Replicate_Movement(ZCom_RSetupMovement<TYPE>* _setup);
  /// constructor, builds the replicator setup automatically
  ZCOM_TAPI ZCom_Replicate_Movement(zU8 _rbits, zU8 _flags, zU8 _rules, zU8 _intercept_id = 0,
    zS16 _mindelay = -1, zS16 _maxdelay = -1);
  ZCOM_TAPI ~ZCom_Replicate_Movement();

  /**
   * @brief Set timescale of movement.
   * @param _scale The time scale.
   *
   * Inform replicator about application's timescale. If this is not set,
   * a scale of 1 is applied, i.e. Zoidcom assumes a physics update frequency 
   * of 1000 hz (a velocity of 5 then equals to 5 units per millisecond = 5000 units per second).
   *
   * Set the scale to your_physics_rate/1000.
   * For a 50hz update frequency, scale should be 0.05. 
   *
   * If your update frequency is fixed at a constant rate, you can leave this value at 1,
   * and call ZCom_Control::processReplicators(1) instead of the actually passed simulation time.
   */
  ZCOM_TAPI void setTimeScale(zFloat _scale);

  /**
   * @brief Set the mandatory update listener.
   * @param _listener Pointer to initialized ZCom_MoveUpdateListener instance.
   *
   * The ZCom_MoveUpdateListener interface informs authority and owner nodes about
   * incoming player input updates and correction feedback. It is necessary
   * for the data flow of this replicator.
   */

  ZCOM_TAPI void setUpdateListener(ZCom_MoveUpdateListener<TYPE> *_listener);

  /**
  * @name Owner data updates and input API.
  *  
  * Feed owner with input data and retrieve past input data to apply corrections from auth.
  * @{ 
  */

  /**
   * @brief Feed input data to owner.
   * @param _pos *Current* position of owner's object. May be NULL.
   * @param _inputstream Bitstream containing application's input data. Like 4 bools
   *                     for (up/down/left/right).
   *
   * The _pos parameter is sent to the authority along with the _inputstream.
   * _pos is not used to update the authority in any way, the authority only needs this data
   * to check if the owner has the correct values. If the sent data differs from the authority's 
   * data by more than the allowed error threshold, the authority will issue a correction on 
   * the owner.
   *
   * Calling this method results in the ZCom_MoveUpdateListener::inputUpdated() callback
   * to be called on the authority. In case of correction, 
   * ZCom_MoveUpdateListener::correctionReceived() will be called later on the owner.
   *
   * @attention Zoidcom takes ownership over the _inputstream bitstream, it may not
   *            be deleted or altered after it has been passed to this method.
   */
  ZCOM_TAPI void updateInput(T *_pos, ZCom_BitStream *_inputstream);

  /**
   * @brief Get the next inputstream that has to be reapplied after the correction.
   * @param _deltatime Pass a pointer to a zU32 here and the method will store the passed
   *                   simulation time between the returned and the previous input stream.
   * @returns Pointer to bitstream originally given to updateInput(). Don't modify or delete it.
   *          If NULL is returned, there is nothing to reapply.
   *
   * When a correction arrives on the owner, the corrected data is data of the past. In fact,
   * the age of the data equals the ping between owner and authority. Therefore, the correction
   * has to be applied in the past. After the correction has been applied, all movement updates
   * that happened after the corrected one, have to be reapplied.
   *
   */
  ZCOM_TAPI ZCom_BitStream* getNextInputHistoryEntry(zU32* _deltatime, void** _iter);

  /** @} 
  */

  /**
   * @name Authority data updates
   *  
   * Update data on authority. 
   * @{ 
   */

  /**
   * @brief Update position, velocity and acceleration.
   * @param _newpos Pointer to array of TYPE[SIZE] containing the new position. May be NULL.
   * @param _newvel Pointer to array of zFloat[SIZE] containing the new velocity. May be NULL.
   * @param _newacc Pointer to array of zFloat[SIZE] containing the new acceleration. May be NULL.
   * @param _teleport If true, tells proxies to apply the new pos directly, instead of interpolating to it.
   * 
   * Call this method on authority and the update will get sent to proxies and owners.
   */
  ZCOM_TAPI void updateState(TYPE *_newpos, zFloat *_newvel, zFloat *_newaccel, bool _teleport);

  /** @} 
   */

  /**
   * @name Proxy data overrides.
   *
   * Temporarily and locally override values on proxy.
   * @{ 
   */

  /**
   * @brief Override position, velocity and acceleration.
   * @param _newpos Pointer to array of TYPE[SIZE] containing the new position.
   * @param _newvel Pointer to array of TYPE[SIZE] containing the new velocity.
   * @param _newacc Pointer to array of TYPE[SIZE] containing the new acceleration.
   * @param _time Relative time. 0 == current simulation time.
   *
   * The physics state will be locally overridden. All consecutive queries to getPosition() etc
   * will return values based on the overridden state, until the next update from the authority
   * arrives.
   */
  ZCOM_TAPI void overrideState(TYPE *_newpos, zFloat *_newvel, zFloat *_newaccel, zS32 _time = 0);

  /** @} 
   */

  /**
   * @name Data fetching.
   *
   * Use these to get the current state.
   * @{ 
   */

  /**
   * @brief Get extrapolated position for given time.
   * @param _attime Time for which the extrapolation is needed. 0 = current simulation time.
   * @param _data Pointer to array where the result will get stored.
   */
  ZCOM_TAPI void getExtrapolatedPosition(zS32 _attime, TYPE *_data);

  /**
   * @brief Get the current velocity used for extrapolation.
   * @param _attime Time for which the extrapolation is needed. 0 = current simulation time.
   * @param _data Pointer to array where the result will get stored.
   */
  ZCOM_TAPI void getExtrapolatedVelocity(zS32 _attime, zFloat *_data);

  /**
   * @brief Get the current acceleration used for extrapolation.
   * @param _data Pointer to array where the result will get stored.
   *
   * Acceleration is constant during extrapolation.
   */
  ZCOM_TAPI void getAcceleration(zFloat *_data);

  /**
   * @brief Get last received position.
   * @param _data Pointer to array where the result will get stored.
   */
  ZCOM_TAPI void getLastReceivedPosition(T *_data);

  /**
  * @brief Get last received velocity.
  * @param _data Pointer to array where the result will get stored.
  */
  ZCOM_TAPI void getLastReceivedVelocity(zFloat *_data);

  /** @} 
   */

  /** 
   * @{
   * @name Debugging
   */

  /**
   * @brief Get a point on the interpolation spline.
   * @param _t Must be between 0 <= t <= 1. 0 gives the start point of the interpolation, 1
   *           the endpoint.
   * @param _data Pointer to an array of TYPE with SIZE elements.
   */
  ZCOM_TAPI void getSplinePoint(zFloat t, TYPE *_data);

  /**
   * @brief Get a control point from the spline.
   * @param _idx Must be 0 <= _idx <= 3.
   * @param _data Pointer to an array of TYPE with SIZE elements.
   */
  ZCOM_TAPI void getSplineControlPoint(zU32 _idx, TYPE *_data);

  /** @}
   */
protected:
  ZCOM_TAPI virtual void onPreSendData(ZCom_ConnID _cid, eZCom_NodeRole _remoterole, zU32 *_lastupdate);
  ZCOM_TAPI virtual void onDataReceived(ZCom_ConnID _cid, eZCom_NodeRole _remoterole, ZCom_BitStream &_stream,
                                       bool _store, zU32 _estimated_time_sent);
  ZCOM_TAPI virtual void onDataLost(ZCom_ConnID _cid, zU32 _reference_id, ZCom_BitStream *_data);
  ZCOM_TAPI virtual void onDataAcked(ZCom_ConnID _cid, zU32 _reference_id, ZCom_BitStream *_data);
  ZCOM_TAPI virtual void onPacketReceived(ZCom_ConnID _cid);
  ZCOM_TAPI virtual void onConnectionAdded(ZCom_ConnID _cid, eZCom_NodeRole _remoterole);
  ZCOM_TAPI virtual void onConnectionRemoved(ZCom_ConnID _cid, eZCom_NodeRole _remoterole);
  ZCOM_TAPI virtual void onLocalRoleChanged(eZCom_NodeRole _oldrole, eZCom_NodeRole _newrole);
  ZCOM_TAPI virtual void onRemoteRoleChanged(ZCom_ConnID _cid, eZCom_NodeRole _oldrole, eZCom_NodeRole _newrole) {};

  ZCOM_TAPI virtual void  Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed);
  ZCOM_TAPI virtual void* peekData();
  ZCOM_TAPI virtual void  clearPeekData() {};
protected:
  ZCom_ReplicateMovementPrivate<T, SIZE> *m_priv;
};

#endif
