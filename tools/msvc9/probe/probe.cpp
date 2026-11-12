// Toolchain probe: STL, Win32 and ATL headers, plus a function to find in the PDB.
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <atlbase.h>

int ProbeSum(const std::vector<int>& v);

std::string ProbeFormat(int x)
{
	std::ostringstream s;
	s << "x=" << x;
	return s.str();
}

int main()
{
	std::map<std::string, int> m;
	std::vector<int> v;
	for (int i = 0; i < 4; i++) {
		v.push_back(i);
		m[ProbeFormat(i)] = i;
	}
	CComBSTR b(L"probe");
	return ProbeSum(v) + (int) m.size() + (int) b.Length() - 15 + (GetCurrentProcessId() == 0);
}
