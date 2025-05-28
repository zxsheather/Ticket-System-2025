#pragma once

#include "../stl/vector.hpp"

template <typename T, typename Comparator = std::less<T>>
void mergeSort(sjtu::vector<T>& arr, int left, int right,
               Comparator comparator = Comparator()) {
  if (left >= right) return;

  int mid = left + (right - left) / 2;
  mergeSort(arr, left, mid, comparator);
  mergeSort(arr, mid + 1, right, comparator);

  T* temp = new T[right - left + 1];
  int i = left, j = mid + 1, k = 0;

  while (i <= mid && j <= right) {
    if (comparator(arr[i], arr[j])) {
      temp[k++] = arr[i++];
    } else {
      temp[k++] = arr[j++];
    }
  }

  while (i <= mid) {
    temp[k++] = arr[i++];
  }

  while (j <= right) {
    temp[k++] = arr[j++];
  }

  for (int idx = 0; idx < right - left + 1; ++idx) {
    arr[left + idx] = temp[idx];
  }

  delete[] temp;
}
