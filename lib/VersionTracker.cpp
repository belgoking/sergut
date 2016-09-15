#include "VersionTracker.h"

#include <sstream>

namespace rsm {

VersionTracker::Entity::Entity(const char* aVersionString, const rsm::VersionTracker::Type aEntityType)
  : versionString(aVersionString), entityType(aEntityType)
{
  VersionTracker::getInstance().registerEntity(*this);
}

::std::string VersionTracker::getAllVersions() const
{
  ::std::ostringstream ostr;
  if(application != nullptr) {
    ostr << application->versionString << "\n";
  }
  for(const Entity* libEntity: libraries) {
    ostr << libEntity->versionString << "\n";
  }
  return ostr.str();
}

VersionTracker& VersionTracker::getInstance()
{
  static VersionTracker instance;
  return instance;
}

void VersionTracker::registerEntity(const VersionTracker::Entity& entity)
{
  switch(entity.entityType) {
  case Application:
    if(application != nullptr) {
      throw "ERROR";
    }
    application = &entity;
    break;
  case Library:
    libraries.push_back(&entity);
    break;
  }
}

}
