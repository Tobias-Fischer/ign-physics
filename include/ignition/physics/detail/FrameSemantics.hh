/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef IGNITION_PHYSICS_DETAIL_FRAMESEMANTICS_HH_
#define IGNITION_PHYSICS_DETAIL_FRAMESEMANTICS_HH_

#include <ignition/physics/FrameSemantics.hh>

namespace ignition
{
  namespace physics
  {
    /////////////////////////////////////////////////
    template <typename FQ>
    typename FQ::Quantity FrameSemantics::Resolve(
        const FQ &_quantity,
        const FrameID _relativeTo,
        const FrameID _inCoordinatesOf) const
    {
      using Quantity = typename FQ::Quantity;
      using Space = typename FQ::Space;
      using FrameDataType = typename Space::FrameDataType;
      using RotationType = typename Space::RotationType;
      using Scalar = typename Space::Scalar;

      const FrameID parentFrame = _quantity.ParentFrame();

      Quantity q;
      RotationType currentCoordinates;

      if (_quantity.ParentFrame() == _relativeTo)
      {
        // The quantity is already expressed relative to the _relativeTo frame

        if (_relativeTo.id == _inCoordinatesOf.id)
        {
          // The quantity is already expressed in coordinates of the
          // _inCoordinatesOf frame
          return _quantity.RelativeToParent();
        }

        q = _quantity.RelativeToParent();
        currentCoordinates = this->FrameDataRelativeToWorld(
              _relativeTo).transform.Rot();
      }
      else
      {
        if (FrameID::World() == _relativeTo)
        {
          // Resolving quantities to the world frame requires fewer operations
          // than resolving to an arbitrary frame, so we use a special function
          // for that.
          q = Space::ResolveToWorldFrame(
                _quantity.RelativeToParent(),
                this->FrameDataRelativeToWorld(parentFrame));

          // The World Frame has all zero fields
          currentCoordinates = RotationType(1.0, 0.0, 0.0, 0.0);
        }
        else
        {
          const FrameDataType relativeToData =
              this->FrameDataRelativeToWorld(_relativeTo);

          q = Space::ResolveToTargetFrame(
                _quantity.RelativeToParent(),
                this->FrameDataRelativeToWorld(parentFrame),
                relativeToData);

          currentCoordinates = relativeToData.transform.Rot();
        }
      }

      if (_relativeTo != _inCoordinatesOf)
      {
        if (FrameID::World().id == _inCoordinatesOf.id)
        {
          // Resolving quantities to the world coordinates requires fewer
          // operations than resolving to an arbitrary frame.
          return Space::ResolveToWorldCoordinates(q, currentCoordinates);
        }
        else
        {
          const RotationType inCoordinatesOfRotation =
              this->FrameDataRelativeToWorld(
                _inCoordinatesOf).transform.Rot();

          return Space::ResolveToTargetCoordinates(
                q, currentCoordinates, inCoordinatesOfRotation);
        }
      }

      return q;
    }

    /////////////////////////////////////////////////
    template <typename FQ>
    typename FQ::Quantity FrameSemantics::Resolve(
        const FQ &_quantity, const FrameID _relativeTo) const
    {
      return this->Resolve(_quantity, _relativeTo, _relativeTo);
    }

    /////////////////////////////////////////////////
    template <typename FQ>
    FQ FrameSemantics::Reframe(
        const FQ &_quantity, const FrameID _withRespectTo) const
    {
      return FQ(_withRespectTo,
                this->Resolve(_quantity, _withRespectTo, _withRespectTo));
    }
  }
}

#endif
