#pragma once

class String;

namespace std
{
template <typename T>
class allocator;

template <typename T, typename Allocator>
class vector;
}  // namespace std

unsigned row_choice(const std::vector<String, std::allocator<String>>&);
String keyboard_input();