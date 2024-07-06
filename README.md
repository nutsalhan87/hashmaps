# Хэш таблицы

Решил разобраться, что из себя представляет одна из самых популярных структур данных. Для этого я решил реализовать их на C.

Каждая таблица предоставляет функции по конструированию, вставке, поиску, удалению, очистке и деконструированию.

## Что реализовано
* Separate chaining - [заголовок](implementations/separate_chaining/hashmap_sc.h)/[реализация](implementations/separate_chaining/hashmap_sc.c)
* Linear probing - [заголовок](implementations/linear_probing/hashmap_lp.h)/[реализация](implementations/linear_probing/hashmap_lp.c)
* Quadratic probing - [заголовок](implementations/quadratic_probing/hashmap_qp.h)/[реализация](implementations/quadratic_probing/hashmap_qp.c)
* Double hashing - [заголовок](implementations/double_hashing/hashmap_dh.h)/[реализация](implementations/double_hashing/hashmap_dh.c)

##  Обертки на других ЯП

Делать тесты производительности на C не очень удобно, а потому я решил написать их на другом языке.

Сначала выбор пал на C++, т.к. ничего особенного для вызова C кода из C++ делать не нужно. Напрямую работать с
указателями, вручную их очищать не круто, а потому написан класс-оболочка с единым интерфейсом, 
который обобщает сразу все реализации хэш таблиц.\
[Реализация класса-оболочки и тестов](performance_test.cpp).

Однако одним C++ сыт не будешь, а потому структура-оболочка и тесты были написаны и на Rust.\
Реализация [структуры-оболочки](performance_test_rs/src/hashmap.rs) и [тестов](performance_test_rs/src/main.rs).

## Тесты

Заголовки столбцов - по первым буквам названий алгоритмов.\
Также в таблице есть STD - это стандартная хэш таблица для каждого языка ([std::unordered_map](https://en.cppreference.com/w/cpp/container/unordered_map) для C++ 
и [std::collections::HashMap](https://doc.rust-lang.org/std/collections/struct.HashMap.html) для Rust).\
Все время в секундах.

<table>
  <tr>
    <td rowspan="2"></td>
    <td colspan="2">STD</td>
    <td colspan="2">SC</td>
    <td colspan="2">LP</td>
    <td colspan="2">QP</td>
    <td colspan="2">DH</td>
  </tr>
  <tr>
    <td>C++</td>
    <td>Rust</td>
    <td>C++</td>
    <td>Rust</td>
    <td>C++</td>
    <td>Rust</td>
    <td>C++</td>
    <td>Rust</td>
    <td>C++</td>
    <td>Rust</td>
  </tr>
  <tr>
    <td>Миллион вставок в новую хэш таблицу</td>
    <td>0.370028</td>
    <td>0.166182</td>
    <td>0.541501</td>
    <td>0.730395</td>
    <td>0.449081</td>
    <td>0.483061</td>
    <td>0.303569</td>
    <td>0.336243</td>
    <td>0.498998</td>
    <td>0.569285</td>
  </tr>
  <tr>
    <td>Миллион вставок в заранее выделенную хэш таблицу</td>
    <td>0.317267</td>
    <td>0.084267</td>
    <td>0.446306</td>
    <td>0.812760</td>
    <td>0.227487</td>
    <td>0.314784</td>
    <td>0.219221</td>
    <td>0.303431</td>
    <td>0.236068</td>
    <td>0.356020</td>
  </tr>
  <tr>
    <td>Очистка хэш таблицы с миллионом элементов</td>
    <td>0.122645</td>
    <td>0.003993</td>
    <td>0.238109</td>
    <td>0.238044</td>
    <td>0.090057</td>
    <td>0.092354</td>
    <td>0.072518</td>
    <td>0.073064</td>
    <td>0.079114</td>
    <td>0.076695</td>
  </tr>
  <tr>
    <td>Последовательное удаление 100 тыс. элементов</td>
    <td>0.008354</td>
    <td>0.004959</td>
    <td>0.011150</td>
    <td>0.021392</td>
    <td>0.006879</td>
    <td>0.017906</td>
    <td>0.007021</td>
    <td>0.018379</td>
    <td>0.012448</td>
    <td>0.023810</td>
  </tr>
  <tr>
    <td>Поиск миллиона элементов</td>
    <td>0.188558</td>
    <td>0.133308</td>
    <td>0.318548</td>
    <td>0.449115</td>
    <td>0.122213</td>
    <td>0.238040</td>
    <td>0.114482</td>
    <td>0.231326</td>
    <td>0.130770</td>
    <td>0.302130</td>
  </tr>
  <tr>
    <td>Поиск миллиона элементов в обратном порядке</td>
    <td>0.188635</td>
    <td>0.131270</td>
    <td>0.319271</td>
    <td>0.448925</td>
    <td>0.121771</td>
    <td>0.231420</td>
    <td>0.114184</td>
    <td>0.231308</td>
    <td>0.132528</td>
    <td>0.302946</td>
  </tr>
</table>
