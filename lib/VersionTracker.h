#pragma once

#include <string>
#include <vector>

namespace rsm {
class VersionTracker {
public:
  enum Type {
    Application,
    Library
  };

  class Entity {
  public:
    Entity(const char* aVersionString, const VersionTracker::Type aEntityType);

  private:
    friend class VersionTracker;
    const char* versionString;
    const VersionTracker::Type entityType;
  };

  ::std::string getAllVersions() const;
  static VersionTracker& getInstance();

private:
  void registerEntity(const Entity& entity);

private:
  friend class Entity;
  const Entity* application = nullptr;
  std::vector<const Entity*> libraries;
};
}

#define VERSION_TRACKER_UNIQUE_NAME_INTERNAL_JOIN( prefix, line) prefix ## line
#define VERSION_TRACKER_UNIQUE_NAME_INTERNAL( prefix, line) VERSION_TRACKER_UNIQUE_NAME_INTERNAL_JOIN( prefix, line)
#define VERSION_TRACKER_UNIQUE_NAME( prefix ) VERSION_TRACKER_UNIQUE_NAME_INTERNAL( prefix, __LINE__)



#define REGISTER_APPLICATION_VERSION(applicationName, version) \
  namespace { \
  const rsm::VersionTracker::Entity VERSION_TRACKER_UNIQUE_NAME( _VERSION_TRACKER_APP ) ( "Application: " applicationName " (Version: " version ")", rsm::VersionTracker::Application ); \
  }

#define REGISTER_LIBRARY_VERSION(libraryName, version) \
  namespace { \
  const rsm::VersionTracker::Entity VERSION_TRACKER_UNIQUE_NAME( _VERSION_TRACKER_LIB ) ( "Library: " libraryName " (Version: " version ")", rsm::VersionTracker::Library ); \
  }
