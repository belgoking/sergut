#include "VersionTracker.h"

#include <catch.hpp>

#include <cctype>
#include <cinttypes>
#include <iostream>
#include <vector>

REGISTER_APPLICATION_VERSION("Test Anwendung", "3.7.0-SNAPSHOT");

REGISTER_LIBRARY_VERSION("Bibliothek1", "1.7.0-SNAPSHOT");

REGISTER_LIBRARY_VERSION("Bibliothek2", "2.7.0-SNAPSHOT");

////////////////////////////////////////////////////////////////////////////////
// check error handling
////////////////////////////////////////////////////////////////////////////////
// this is a prequel to the checks for invalid XML
TEST_CASE("TestVersionTracker", "[VersionTracker]")
{
  CHECK(std::string("Application: Test Anwendung (Version: 3.7.0-SNAPSHOT)\n"
                    "Library: Bibliothek1 (Version: 1.7.0-SNAPSHOT)\n"
                    "Library: Bibliothek2 (Version: 2.7.0-SNAPSHOT)\n")
        == rsm::VersionTracker::getInstance().getAllVersions());
}
