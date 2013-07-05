/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreSceneNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"
#include "OgreWireBoundingBox.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
							SceneNode *parent )
        : Node( id, nodeMemoryManager, parent )
        , mCreator(creator)
        , mYawFixed(false)
        , mAutoTrackTarget(0)
    {
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
		if( mListener )
			mCreator->unregisterSceneNodeListener( this );

        // Detach all objects
		detachAllObjects();
    }
    //-----------------------------------------------------------------------
    void SceneNode::attachObject(MovableObject* obj)
    {
        if (obj->isAttached())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Object already attached to a SceneNode or a Bone",
                "SceneNode::attachObject");
        }

        obj->_notifyAttached(this);

        // Also add to name index
		mAttachments.push_back( obj );
    }
    //-----------------------------------------------------------------------
	SceneNode::ObjectVec::iterator SceneNode::getAttachedObjectIt( const String& name )
	{
		ObjectVec::iterator itor = mAttachments.begin();
		ObjectVec::iterator end  = mAttachments.end();

		while( itor != end )
		{
			if( (*itor)->getName() == name )
				return itor;
			++itor;
		}

		return end;
	}
	//-----------------------------------------------------------------------
	SceneNode::ObjectVec::const_iterator SceneNode::getAttachedObjectIt( const String& name ) const
	{
		ObjectVec::const_iterator itor = mAttachments.begin();
		ObjectVec::const_iterator end  = mAttachments.end();

		while( itor != end )
		{
			if( (*itor)->getName() == name )
				return itor;
			++itor;
		}

		return end;
	}
	//-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject( const String& name )
    {
		ObjectVec::const_iterator itor = getAttachedObjectIt( name );

		if( itor == mAttachments.end() )
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Attached object " + 
					name + " not found.", "SceneNode::getAttachedObject");
		}

        return *itor;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject( size_t index )
    {
		ObjectVec::iterator itor = mAttachments.begin() + index;
		(*itor)->_notifyAttached( (SceneNode*)0 );
		MovableObject *retVal = *itor;

		efficientVectorRemove( mAttachments, itor );

		return retVal;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject( const String& name )
    {
		MovableObject *retVal = 0;
		ObjectVec::iterator itor = getAttachedObjectIt( name );

		if( itor != mAttachments.end() )
		{
			retVal = *itor;
			(*itor)->_notifyAttached( (SceneNode*)0 );
			efficientVectorRemove( mAttachments, itor );
		}
		else
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Object pointer was not attached to "
				"this scene node", "SceneNode::detachObject");
		}

		return retVal;
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachObject( MovableObject* obj )
    {
		ObjectVec::iterator itor = std::find( mAttachments.begin(), mAttachments.end(), obj );

		if( itor != mAttachments.end() )
		{
			(*itor)->_notifyAttached( (SceneNode*)0 );
			efficientVectorRemove( mAttachments, itor );
		}
		else
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Object pointer was not attached to "
				"this scene node", "SceneNode::detachObject");
		}
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllObjects(void)
    {
		ObjectVec::iterator itr;
		for ( itr = mAttachments.begin(); itr != mAttachments.end(); ++itr )
			(*itr)->_notifyAttached( (SceneNode*)0 );
        mAttachments.clear();
    }
    //-----------------------------------------------------------------------
    /*void SceneNode::_findVisibleObjects(Camera* cam, RenderQueue* queue, 
		VisibleObjectsBoundsInfo* visibleBounds, bool includeChildren, 
		bool displayNodes, bool onlyShadowCasters)
    {
        // Check self visible
        if (!cam->isVisible(mWorldAABB))
            return;

        // Add all entities
        ObjectMap::iterator iobj;
        ObjectMap::iterator iobjend = mObjectsByName.end();
        for (iobj = mObjectsByName.begin(); iobj != iobjend; ++iobj)
        {
			MovableObject* mo = iobj->second;

			queue->processVisibleObject(mo, cam, onlyShadowCasters, visibleBounds);
        }

        if (includeChildren)
        {
            ChildNodeMap::iterator child, childend;
            childend = mChildren.end();
            for (child = mChildren.begin(); child != childend; ++child)
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(child->second);
                sceneChild->_findVisibleObjects(cam, queue, visibleBounds, includeChildren, 
					displayNodes, onlyShadowCasters);
            }
        }

        if (displayNodes)
        {
            // Include self in the render queue
            queue->addRenderable(getDebugRenderable());
        }

		// Check if the bounding box should be shown.
		// See if our flag is set or if the scene manager flag is set.
		if ( !mHideBoundingBox &&
             (mShowBoundingBox || (mCreator && mCreator->getShowBoundingBoxes())) )
		{ 
			_addBoundingBoxToQueue(queue);
		}


    }*/

	/*TODO
	Node::DebugRenderable* SceneNode::getDebugRenderable()
	{
		Vector3 hs = mWorldAABB.getHalfSize();
		Real sz = std::min(hs.x, hs.y);
		sz = std::min(sz, hs.z);
		sz = std::max(sz, (Real)1.0);
		return Node::getDebugRenderable(sz);
	}*/
    //-----------------------------------------------------------------------
    void SceneNode::updateFromParentImpl(void)
    {
        Node::updateFromParentImpl();

        // Notify objects that it has been moved
		ObjectVec::iterator itor = mAttachments.begin();
		ObjectVec::iterator end  = mAttachments.end();

		while( itor != end )
		{
			(*itor)->_notifyMoved();
			++itor;
		}
    }
    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl(void)
    {
        assert(mCreator);
        return mCreator->_createSceneNode( this );
    }
    //-----------------------------------------------------------------------
    SceneNode::ObjectIterator SceneNode::getAttachedObjectIterator(void)
    {
        return ObjectIterator( mAttachments.begin(), mAttachments.end() );
    }
	//-----------------------------------------------------------------------
	SceneNode::ConstObjectIterator SceneNode::getAttachedObjectIterator(void) const
	{
		return ConstObjectIterator( mAttachments.begin(), mAttachments.end() );
	}
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild( SceneNode *sceneNode )
    {
		assert( sceneNode->getParent() == this );
        sceneNode->removeAndDestroyAllChildren();

        removeChild( sceneNode );
        sceneNode->getCreator()->destroySceneNode( sceneNode );
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyAllChildren(void)
    {
		NodeVec::iterator itor = mChildren.begin();
		NodeVec::iterator end  = mChildren.end();
		while( itor != end )
		{
			SceneNode *sceneNode = static_cast<SceneNode*>( *itor );
			sceneNode->removeAndDestroyAllChildren();
			sceneNode->setParent( 0 );
			mCreator->destroySceneNode( sceneNode );
			++itor;
		}

	    mChildren.clear();
    }
    //-----------------------------------------------------------------------
	SceneNode* SceneNode::createChildSceneNode(const Vector3& inTranslate, 
        const Quaternion& inRotate)
	{
		return static_cast<SceneNode*>(this->createChild(inTranslate, inRotate));
	}
	//-----------------------------------------------------------------------
	void SceneNode::setListener( Listener* listener )
	{
		Listener *oldListener = mListener;
		Node::setListener( listener );

		//Un/Register ourselves as a listener in the scene manager
		if( oldListener )
			mCreator->unregisterSceneNodeListener( this );
		if( mListener )
			mCreator->registerSceneNodeListener( this );
	}
    //-----------------------------------------------------------------------
    void SceneNode::findLights(LightList& destList, Real radius, uint32 lightMask) const
    {
        // No any optimisation here, hope inherits more smart for that.
        //
        // If a scene node is static and lights have moved, light list won't change
        // can't use a simple global boolean flag since this is only called for
        // visible nodes, so temporarily visible nodes will not be updated
        // Since this is only called for visible nodes, skip the check for now
        //
        if (mCreator)
        {
            // Use SceneManager to calculate
            mCreator->_populateLightList(this, radius, destList, lightMask);
        }
        else
        {
            destList.clear();
        }
    }
    //-----------------------------------------------------------------------
    void SceneNode::setAutoTracking(bool enabled, SceneNode* const target, 
        const Vector3& localDirectionVector,
        const Vector3& offset)
    {
        if (enabled)
        {
            mAutoTrackTarget = target;
            mAutoTrackOffset = offset;
            mAutoTrackLocalDirection = localDirectionVector;
        }
        else
        {
            mAutoTrackTarget = 0;
        }
        if (mCreator)
            mCreator->_notifyAutotrackingSceneNode(this, enabled);
    }
    //-----------------------------------------------------------------------
    void SceneNode::setFixedYawAxis(bool useFixed, const Vector3& fixedAxis)
    {
        mYawFixed = useFixed;
        mYawFixedAxis = fixedAxis;
    }

	//-----------------------------------------------------------------------
	void SceneNode::yaw(const Radian& angle, TransformSpace relativeTo)
	{
		if (mYawFixed)
		{
			rotate(mYawFixedAxis, angle, relativeTo);
		}
		else
		{
			rotate(Vector3::UNIT_Y, angle, relativeTo);
		}

	}
    //-----------------------------------------------------------------------
    void SceneNode::setDirection(Real x, Real y, Real z, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        setDirection(Vector3(x,y,z), relativeTo, localDirectionVector);
    }

    //-----------------------------------------------------------------------
    void SceneNode::setDirection(const Vector3& vec, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        // Do nothing if given a zero vector
        if (vec == Vector3::ZERO) return;

        // The direction we want the local direction point to
        Vector3 targetDir = vec.normalisedCopy();

		const bool inheritOrientation = mTransform.mInheritOrientation[mTransform.mIndex];

        // Transform target direction to world space
        switch (relativeTo)
        {
        case TS_PARENT:
            if( inheritOrientation )
            {
                if (mParent)
                {
                    targetDir = mParent->_getDerivedOrientation() * targetDir;
                }
            }
            break;
        case TS_LOCAL:
            targetDir = _getDerivedOrientation() * targetDir;
            break;
        case TS_WORLD:
            // default orientation
            break;
        }

        // Calculate target orientation relative to world space
        Quaternion targetOrientation;
        if( mYawFixed )
        {
            // Calculate the quaternion for rotate local Z to target direction
            Vector3 xVec = mYawFixedAxis.crossProduct( targetDir );
            xVec.normalise();
            Vector3 yVec = targetDir.crossProduct(xVec);
            yVec.normalise();
            Quaternion unitZToTarget = Quaternion(xVec, yVec, targetDir);

            if (localDirectionVector == Vector3::NEGATIVE_UNIT_Z)
            {
                // Specail case for avoid calculate 180 degree turn
                targetOrientation =
                    Quaternion(-unitZToTarget.y, -unitZToTarget.z, unitZToTarget.w, unitZToTarget.x);
            }
            else
            {
                // Calculate the quaternion for rotate local direction to target direction
                Quaternion localToUnitZ = localDirectionVector.getRotationTo(Vector3::UNIT_Z);
                targetOrientation = unitZToTarget * localToUnitZ;
            }
        }
        else
        {
            const Quaternion& currentOrient = _getDerivedOrientation();

            // Get current local direction relative to world space
            Vector3 currentDir = currentOrient * localDirectionVector;

            if ((currentDir+targetDir).squaredLength() < 0.00005f)
            {
                // Oops, a 180 degree turn (infinite possible rotation axes)
                // Default to yaw i.e. use current UP
                targetOrientation =
                    Quaternion(-currentOrient.y, -currentOrient.z, currentOrient.w, currentOrient.x);
            }
            else
            {
                // Derive shortest arc to new direction
                Quaternion rotQuat = currentDir.getRotationTo(targetDir);
                targetOrientation = rotQuat * currentOrient;
            }
        }

        // Set target orientation, transformed to parent space
        if( mParent && inheritOrientation )
            setOrientation(mParent->_getDerivedOrientation().UnitInverse() * targetOrientation);
        else
            setOrientation(targetOrientation);
    }
    //-----------------------------------------------------------------------
    void SceneNode::lookAt( const Vector3& targetPoint, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        // Calculate ourself origin relative to the given transform space
        Vector3 origin;
        switch (relativeTo)
        {
        default:    // Just in case
        case TS_WORLD:
            origin = _getDerivedPosition();
            break;
        case TS_PARENT:
			mTransform.mPosition->getAsVector3( origin, mTransform.mIndex );
            break;
        case TS_LOCAL:
            origin = Vector3::ZERO;
            break;
        }

        setDirection( targetPoint - origin, relativeTo, localDirectionVector );
    }
    //-----------------------------------------------------------------------
    void SceneNode::_autoTrack(void)
    {
        // NB assumes that all scene nodes have been updated
        if (mAutoTrackTarget)
        {
            lookAt( mAutoTrackTarget->_getDerivedPosition() + mAutoTrackOffset, 
					TS_WORLD, mAutoTrackLocalDirection );
        }
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::getParentSceneNode(void) const
    {
        return static_cast<SceneNode*>(getParent());
    }
    //-----------------------------------------------------------------------
    void SceneNode::setVisible( bool visible, bool cascade )
    {
		ObjectVec::iterator itor = mAttachments.begin();
		ObjectVec::iterator end  = mAttachments.end();

		while( itor != end )
		{
			(*itor)->setVisible( visible );
			++itor;
		}

        if (cascade)
        {
			NodeVec::iterator itor = mChildren.begin();
			NodeVec::iterator end  = mChildren.end();
			while( itor != end )
			{
				static_cast<SceneNode*>( *itor )->setVisible( visible, cascade );
				++itor;
			}
        }
    }
	//-----------------------------------------------------------------------
	void SceneNode::setDebugDisplayEnabled(bool enabled, bool cascade)
	{
		ObjectVec::iterator itor = mAttachments.begin();
		ObjectVec::iterator end  = mAttachments.end();

		while( itor != end )
		{
			(*itor)->setDebugDisplayEnabled( enabled );
			++itor;
		}

		if (cascade)
		{
			NodeVec::iterator itor = mChildren.begin();
			NodeVec::iterator end  = mChildren.end();
			while( itor != end )
			{
				static_cast<SceneNode*>( *itor )->setDebugDisplayEnabled( enabled, cascade );
				++itor;
			}
		}
	}
    //-----------------------------------------------------------------------
    void SceneNode::flipVisibility(bool cascade)
    {
		ObjectVec::iterator itor = mAttachments.begin();
		ObjectVec::iterator end  = mAttachments.end();

		while( itor != end )
		{
			(*itor)->setVisible( (*itor)->getVisible() );
			++itor;
		}

        if (cascade)
        {
			NodeVec::iterator itor = mChildren.begin();
			NodeVec::iterator end  = mChildren.end();
			while( itor != end )
			{
				static_cast<SceneNode*>( *itor )->flipVisibility( cascade );
				++itor;
			}
        }
    }
}
