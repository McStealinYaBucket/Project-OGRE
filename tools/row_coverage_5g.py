"""Row 5g coverage (Http, content providers, asset fetch): tools/row_coverage.py with the 5g file set.

    python3 -P tools/row_coverage_5g.py P.json [--csv out.csv]

Same method and caveats as row_coverage.py; only the file -> class-qualifier table differs.
WinHttp.cpp / WinInet.cpp (the only WINHTTP/WININET IAT callers) have no oracle names and keep their
template markers in Util/Http.h, so their .obj ranges are fixed: WinHttp 0x008239b0-0x00827510
(the http_status_error copy ctor at 0x00827510 is its last COMDAT), WinInet 0x00827540-0x00828250.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import row_coverage as rc  # noqa: E402

ROW_5G = {
    "Client/App/util/ContentId.cpp": ["RBX::ContentId::"],
    "Client/App/util/LegacyContentTable.cpp": ["RBX::LegacyContentTable::"],
    "Client/App/util/ContentProvider.cpp": ["RBX::ContentProvider::", "RBX::parseContent"],
    "Client/App/util/ContentProviderJob.cpp": ["RBX::ContentProviderJob::"],
    "Client/App/util/CacheableContentProvider.cpp": ["RBX::CacheableContentProvider::",
                                                     "RBX::TextureContentProvider::",
                                                     "RBX::MeshContentProvider::"],
    "Client/App/util/Http.cpp": ["RBX::Http::"],
    "Client/App/util/WinHttp.cpp": ["RBX::Http::httpGetPostWinHttp"],
    "Client/App/util/WinInet.cpp": ["RBX::Http::httpGetPostWinInet"],
    "Client/App/util/AsyncHttpQueue.cpp": ["RBX::AsyncHttpQueue::", "RBX::HttpQueueStatsItem::"],
    "Client/App/v8datamodel/InsertService.cpp": ["RBX::InsertService::"],
    "Client/App/v8datamodel/MarketplaceService.cpp": ["RBX::MarketplaceService::"],
}

# ContentProvider.obj's markers have a 0x3470 gap (0x006623a0 -> 0x00665810); widen the cluster span.
rc.ROWS["5g"] = ROW_5G
rc.FIXED["Client/App/util/WinHttp.cpp"] = (0x008239b0, 0x00827510)
rc.FIXED["Client/App/util/WinInet.cpp"] = (0x00827540, 0x00828250)
rc.SPAN["5g"] = 0x4000
if "--row" not in sys.argv:
    sys.argv[2:2] = ["--row", "5g"]

if __name__ == "__main__":
    sys.exit(rc.main())
