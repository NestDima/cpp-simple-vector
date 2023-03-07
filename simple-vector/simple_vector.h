#pragma once
 
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include "array_ptr.h"


class ReserveProxyObj {
public:    
    explicit ReserveProxyObj (size_t capacity_to_reserve) 
        : capacity_(capacity_to_reserve){}

    size_t Reserve_capacity() {
        return capacity_;
    }

private:    
    size_t capacity_;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
    : SimpleVector(size, std::move(Type{})){}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
    : simple_vector_(size)
    , size_(size)
    , capacity_(size){
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
    : simple_vector_(init.size())
    , size_(init.size())
    , capacity_(init.size()){
        std::copy(init.begin(), init.end(), begin());
    }
    
    SimpleVector(ReserveProxyObj capacity_to_reserve) {
        Reserve(capacity_to_reserve.Reserve_capacity());
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return !size_;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return simple_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return simple_vector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("The index is out of the range");
        }
        else{
            return simple_vector_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("The index is out of the range");
        }
        else{
            return simple_vector_[index];
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_){
            size_ = new_size;
            return;
        }
        else if(new_size<capacity_){
            for (auto iter = begin() + new_size; iter != end(); --iter) {
                *iter = std::move(Type{});
            }
            size_ = new_size;
            return;
        }
        else{
            ArrayPtr<Type> temp(new_size);
            std::move(begin(), end(), &temp[0]);
            simple_vector_.swap(temp);
            size_ = new_size;
            capacity_ = new_size*2;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator(&simple_vector_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator(&simple_vector_[size_]);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(&simple_vector_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator(&simple_vector_[size_]);
    }
    
    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> temp(other.size_);
        size_ = other.size_;
        capacity_= other.capacity_;
        std::copy(other.begin(), other.end(), &temp[0]);
        simple_vector_.swap(temp);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this !=&rhs){
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) 
    : simple_vector_(other.size_) {
        size_ = std::move(other.size_);
        capacity_ = std::move(other.capacity_);
        simple_vector_.swap(other.simple_vector_);
        other.Clear();
    }
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (capacity_ > size_) {
            simple_vector_[size_++] = item;
            return;
        }
        if (capacity_) {
            ArrayPtr<Type> temp(capacity_*=2);
            std::copy(begin(), end(), &temp[0]);
            simple_vector_.swap(temp);
            simple_vector_[size_++] = item;
        }
        else {
            ArrayPtr<Type> temp(++capacity_);
            std::copy(begin(), end(), &temp[0]);
            simple_vector_.swap(temp);
            simple_vector_[size_++] = item;
            return;
        }
    }

    // Добавляет элемент в конец вектора
    // move-семантика
    void PushBack(Type&& item) {
        if (capacity_ > size_) {
            simple_vector_[size_++] = std::move(item);
            return;
        }
        if (capacity_) {
            ArrayPtr<Type> temp(capacity_*=2);
            std::move(begin(), end(), &temp[0]);
            simple_vector_.swap(temp);
            simple_vector_[size_++] = std::move(item);
        }
        else {
            ArrayPtr<Type> temp(++capacity_);
            std::move(begin(), end(), &temp[0]);
            simple_vector_.swap(temp);
            simple_vector_[size_++] = std::move(item);
            return;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        if(begin()<= pos && end()>= pos){
        auto index = std::distance(cbegin(), pos);
            if (size_== capacity_){
                if(size_){
                    ArrayPtr<Type> temp(size_);
                    std::copy(begin(), end(), &temp[0]);
                    simple_vector_.swap(temp);
                    size_ = size_;
                    capacity_ = size_*2;
                }
                else{
                    ArrayPtr<Type> temp(1);
                    std::copy(begin(), end(), &temp[0]);
                    simple_vector_.swap(temp);
                    capacity_=1;
                }
            }
            for (size_t i = size_; i > (size_t)index; --i) {
                simple_vector_[i] = simple_vector_[i-1];
            }
            ++size_;
            simple_vector_[index] = value;
            return const_cast<Iterator>(index+begin()); 
        }
        else{
            throw std::out_of_range("The index is out of range");
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // move-семантика
    Iterator Insert(ConstIterator pos, Type&& value) {
        if(begin()<= pos && end()>= pos){
            if (capacity_ == 0){
                ArrayPtr<Type> temp(++capacity_);
                std::move(begin(), end(), &temp[0]);
                simple_vector_.swap(temp);
                simple_vector_[size_++] = std::move(value);
                return begin();
            }
            else if (capacity_ <= size_){
                auto index = std::distance(begin(), const_cast<Iterator>(pos));
                ArrayPtr<Type> temp(capacity_*=2);
                std::move(begin(), end(), &temp[0]);
                std::copy_backward(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(begin()+size_), (&temp[size_+1]));
                temp[index] = std::move(value);
                ++size_;
                simple_vector_.swap(temp);
                return Iterator(&simple_vector_[index]);
            }
            else{
                std::copy_backward(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(end()), (&simple_vector_[++size_+1]));
                *const_cast<Iterator>(pos) = std::move(value);
                return const_cast<Iterator>(pos);
            }

        }
        else{
            throw std::out_of_range("The index is out of range");
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(size_) --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin());
        assert(pos <= end());
        auto index = std::distance(cbegin(), pos);
        std::move(&simple_vector_[index + 1], end(), const_cast<Iterator>(pos));
        --size_;
        return const_cast<Iterator>(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        simple_vector_.swap(other.simple_vector_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    void Reserve(size_t new_capacity){
        if (new_capacity > capacity_) {
            ArrayPtr<Type> temp(new_capacity);
            std::copy(begin(),end(),&temp[0]);
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
        else{
            return;
        }
    }
    
private:
    ArrayPtr<Type> simple_vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs==rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    return true;
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 