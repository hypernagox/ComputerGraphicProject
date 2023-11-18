#pragma once

template <typename T> requires std::is_enum<T>::value
constexpr int etoi(T _eType) { return static_cast<int>(_eType); }
class GameObj;

template<typename T>
const ushort RemoveElement(vector<T>& _delVec,const ushort _delIdx)noexcept {
	_delVec[_delIdx] = std::move(_delVec.back());
	_delVec.pop_back();
	return _delIdx;
}

template<>
const ushort RemoveElement<shared_ptr<GameObj>>(vector<shared_ptr<GameObj>>& _delVec, const ushort _delIdx)noexcept;


template<typename T,typename Fp>
void RemoveElementAll(vector<T>& _delVec,Fp _bFp)
{
	for (ushort idx = 0; idx < (const ushort)_delVec.size();)
	{
		if (_bFp(_delVec[idx]))
		{
			idx = RemoveElement(_delVec, idx);
		}
		else
		{
			++idx;
		}
	}
}

template <typename T>
concept ContiguousContainer = requires(T a) {
	{ a.data() } -> std::same_as<typename T::value_type*>;
	{ a.size() } -> std::convertible_to<std::size_t>;
};

template <typename T,typename Compare> requires ContiguousContainer<T>
void insertionSort(T& arr_,Compare cmp_)
{
	using ele = std::remove_cvref_t<decltype(arr_[0])>;
	std::span<ele> arr_s = arr_;
	const int n = (int)arr_s.size();
	auto arr = arr_s.data();
	std::byte key[sizeof(ele)];
	for (int i = n - 2; i >= 0; --i) {
		memcpy(key, arr + i, sizeof(ele));
		int j = i + 1;
		while (j < n && cmp_(arr[j], *reinterpret_cast<ele*>(key))) {
			++j;
		}
		if (j != i + 1) {
			memmove(arr + i, arr + i + 1, (j - i - 1) * sizeof(ele));
			memcpy(arr + j - 1, key, sizeof(ele));
		}
	}
}


glm::vec3 RandomColor();

class CoRoutine;
class WaitForSeconds;

bool WaitSecondsHelper(WaitForSeconds*& _pWait)noexcept;

void StartCoRoutine(string_view _strKey, CoRoutine&& _coTask);

void StartCoRoutine(CoRoutine&& _coTask);

bool IsCollision2D(const glm::vec2& A_LT, const glm::vec2& A_RB, const glm::vec2& B_LT, const glm::vec2& B_RB);

std::tuple<glm::vec3, glm::quat, glm::vec3> Decompose(const glm::mat4& transformMat_)noexcept;

const glm::vec3 ScreenToOpenGL2D(const glm::vec2& mpos)noexcept;

const glm::vec2 OpenGL2D2Screen(const glm::vec3& glpos)noexcept;

std::wstring string2wstring(std::string_view input)noexcept;

std::string wstring2string(std::wstring_view input)noexcept;

glm::vec3 GetMaxXYZ(const glm::vec3 v)noexcept;