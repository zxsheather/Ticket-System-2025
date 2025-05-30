# 火车票订票系统总体设计文档

## 1. 项目概述

本项目实现一个类似于12306的火车票订票系统，提供车票查询、购票、订单操作等用户功能，以及后台管理功能。系统在本地存储用户数据、购票数据、车次数据，并对其进行高效操作。

### 1.1 系统目标

- 支持用户管理：注册、登录、查询、修改用户信息
- 支持车次管理：添加、删除、发布、查询车次信息
- 支持票务操作：查询车票、购买车票、候补购票
- 支持订单管理：查询订单、退票
- 在内存限制下高效处理大量数据，支持程序多次启动关闭

### 1.2 技术要求与约束

- 编程语言：C++
- 允许使用的标准库：iostream, string, cstdio, cmath, string, fstream, filesystem
- 不允许使用STL容器(除std::string外)或算法
- 内存使用严格限制，不能将全部数据存入内存
- 程序支持多次启动关闭，数据需持久化存储
- 最多创建50个文件

## 2. 系统架构设计

系统采用分层架构，从底向上分为：

### 2.1 分层结构

```
+-------------------+
|   交互层 (CLI)    |  处理命令解析与结果输出
+-------------------+
|   业务逻辑层      |  实现各类指令的业务逻辑
+-------------------+
|   数据访问层      |  提供数据结构和存储访问接口
+-------------------+
|   存储引擎层      |  实现数据持久化和索引机制
+-------------------+
```

### 2.2 核心模块划分

1. **交互模块**：解析用户输入的命令，分发给对应的业务逻辑处理器
2. **用户管理模块**：处理用户相关操作，包括登录、注册、查询、修改等
3. **车次管理模块**：处理车次相关操作，包括添加、删除、发布、查询等
4. **票务管理模块**：处理票务相关操作，包括直达车票查询、复杂换乘查询、购票、候补等
5. **订单管理模块**：处理订单相关操作，包括查询订单、退票等
6. **存储引擎模块**：包括B+树索引、缓存管理、文件读写等功能

## 3. 数据结构设计

### 3.1 自实现的STL替代容器

基于项目STL限制要求，我们实现了以下容器：

- **vector**：动态数组，支持随机访问和动态扩容
- **list**：双向链表，支持高效插入和删除  
- **map**：有序映射，基于红黑树实现
- **pair**：键值对工具类
- **priority_queue**：优先队列，用于候补订单系统（待实现）
- **FixedString<N>**：固定长度字符串，避免内存碎片化

这些容器位于`src/stl/`目录下，提供与标准STL类似的接口，但针对项目需求进行了优化。

### 3.2 业务数据结构

#### 3.2.1 用户数据结构

```cpp
// 用户核心数据结构
struct User {
    FixedString<20> username;    // 用户名，唯一标识符
    FixedString<30> password;    // 密码
    FixedString<20> name;        // 用户真实姓名
    FixedString<30> mail_addr;   // 邮箱地址
    int privilege;               // 用户权限级别(0-10)
    
    // 构造函数
    User(const std::string& username, const std::string& password, 
         const std::string& name, const std::string& mail_addr, const int& privilege);
         
    // 比较运算符 <, >, ==, !=, <=, >=
};

// 用户配置文件，用于queryProfile和modifyProfile返回
struct UserProfile {
    std::string username;   // 用户名
    std::string name;       // 真实姓名
    std::string mail_addr;  // 邮箱地址
    int privilege;          // 权限级别
};
```

#### 3.2.2 车次数据结构

```cpp
struct Train {
    FixedString<20> train_id;               // 车次唯一标识符
    int station_num;                        // 车站数量
    FixedString<30> stations[MAX_STATION_NUM]; // 各站名称
    int seat_num;                           // 座位数量
    int prices[MAX_STATION_NUM - 1];        // 相邻站点间票价
    Time arrival_times[MAX_STATION_NUM];    // 各站到达时间
    Time departure_times[MAX_STATION_NUM];  // 各站出发时间
    Date sale_date_start;                   // 售票开始日期
    Date sale_date_end;                     // 售票结束日期
    char type;                              // 列车类型
    bool is_released;                       // 是否已发布
    int seat_map_pos;                     // 座位图位置(在seat.memoryriver中的偏移量)
};

// 时间相关数据结构
struct Date {
    int month, day;
    std::string toString() const;
    Date operator+(int days) const;
    ... // 其他日期操作方法
};

struct Time {
    int hour, minute;
    std::string toString() const;
    Time operator+(int minutes) const;
    ... // 其他时间操作方法
};

struct TimePoint {
    Date date;
    Time time;
    std::string toString() const;
    TimePoint operator+(const Time& other) const;
    TimePoint operator+(int minutes) const;
    ... // 其他时间点操作方法
};

// 车次+日期的复合键，用于座位管理
struct UniTrain {
    FixedString<20> train_id;
    Date date;
    // 比较运算符 <, >, ==, !=, <=, >=
};
```

#### 3.2.3 座位管理数据结构

**当前实现的座位管理**：

```cpp
struct SeatMap {
    int total_seats;                      // 总座位数
    int station_num;                      // 站点数量  
    int seat_num[MAX_STATION_NUM];        // 各区间剩余座位数量
    
    // 查询可用座位数量
    int queryAvailableSeat(int start_station, int end_station);
    
    // 检查座位是否可用
    bool isSeatAvailable(int start_station, int end_station, int seat);
    
    // 预订座位方法（在seat.hpp中实现）
    bool bookSeat(int start_station, int end_station, int seat);
    
    // 释放座位方法
    void releaseSeat(int start_station, int end_station, int seat);
    
    // 比较运算符
    bool operator==(const SeatMap& other) const;
    bool operator!=(const SeatMap& other) const;
    bool operator<(const SeatMap& other) const;
    bool operator>(const SeatMap& other) const;
    bool operator<=(const SeatMap& other) const;
    bool operator>=(const SeatMap& other) const;
};
```

**SeatManager实现**（位于`controller/seat_manager.cpp`）：
```cpp
class SeatManager {
private:
    MemoryRiver<SeatMap> seat_db;  // 使用MemoryRiver进行座位存储

public:
    // 查询座位分布
    SeatMap querySeat(int start_pos, int& seat_map_pos, int date_from_sale_start);
    
    // 预订座位
    int bookSeat(int seat_map_pos, int start_station, int end_station, 
                 int seat, SeatMap& seat_map);
    
    // 初始化座位
    void initSeat(const Train& train, int& train_seat);
    
    // 释放座位
    void releaseSeat(int seat_map_pos, int start_station, int end_station,
                     int seat, SeatMap& seat_map);
};
```

**座位存储策略**：
- 直接存储各区间剩余座位数量
- 一个车次的所有销售日期的座位图连续存储
- 位置计算：`seat_map_pos = start_pos + date_from_sale_start * sizeof(SeatMap)`
- 支持原地更新，避免频繁的文件重写

#### 3.2.4 订单数据结构

```cpp
enum OrderStatus { SUCCESS, PENDING, REFUNDED };

struct Order {
    FixedString<20> username;         // 用户名
    FixedString<20> train_id;         // 车次ID
    FixedString<30> from_station;     // 出发站
    FixedString<30> to_station;       // 到达站
    Date date;                        // 乘车日期
    Time leaving_time;                // 出发时间
    Time arriving_time;               // 到达时间
    int price;                        // 总票价
    int num;                          // 购票数量
    OrderStatus status;               // 订单状态：SUCCESS, PENDING, REFUNDED
    int timestamp;                    // 创建时间戳，用于订单排序和去重

    ... // 构造函数、比较运算符等方法
    
};
```

**设计特点**：

1. **固定长度字符串**：使用`FixedString`替代`std::string`，避免动态内存分配，提高序列化效率
2. **类型化时间**：使用`Date`和`Time`类型替代字符串，支持高效的时间计算和比较
3. **基于时间戳排序**：订单按创建时间戳排序，便于查询和候补队列管理
4. **双重存储策略**：
   - 正常订单：以用户名为键存储在`order_db`中
   - 候补订单：以车次+日期的哈希值为键存储在`pending_db`中
5. **状态管理**：支持三种状态转换：`PENDING` → `SUCCESS` 或 `REFUNDED`

### 3.3 索引结构

系统使用B+树实现关键索引，核心索引包括：

1. **用户索引**：`BPT<uint64_t, User>` - 使用用户名哈希值作为键
2. **车次索引**：`BPT<FixedString<20>, Train>` - 使用车次ID作为键
3. **站点索引**：`BPT<FixedString<30>, FixedString<20>>` - 站点名 -> 车次ID列表
4. **路线索引**：`BPT<Route, FixedString<20>>` - 路线信息 -> 车次ID
5. **订单索引**：`BPT<FixedString<20>, Order>` - 用户名 -> 订单列表
6. **候补队列索引**：`BPT<long long, Order>` - 车次+日期哈希 -> 候补订单队列

**座位管理存储**：
- 使用MemoryRiver直接文件访问，不使用B+树索引
- 连续存储模式：同一车次不同日期的座位图顺序存放
- 通过偏移计算快速定位：`seat_map_pos = start_pos + date_offset * sizeof(SeatMap)`

所有索引都使用`Hash::hashKey`函数生成哈希键，支持中文字符串的高效哈希。

## 4. 存储系统设计

### 4.1 B+树索引

B+树是系统的核心索引结构，支持模板化的键值对存储。主要接口：

```cpp
template<class Key, class Value>
class BPT {
public:
    void insert(const Key &key, const Value &value);  // 插入键值对
    void remove(const Key &key, const Value &value);  // 删除键值对
    sjtu::vector<Value> find(const Key &key);         // 查找key对应的所有值
    bool empty();                                      // 判断树是否为空
    bool exists(const Key &key);                       // 判断key是否存在
};
```

**实例化的B+树类型**（在`bplus_tree.cpp`中实现）：
- `BPT<uint64_t, User>` - 用户管理（使用哈希键）
- `BPT<FixedString<20>, Train>` - 车次管理  
- `BPT<FixedString<30>, FixedString<20>>` - 站点名到车次ID映射
- `BPT<Route, FixedString<20>>` - 路线到车次映射
- `BPT<FixedString<20>, Order>` - 用户订单管理
- `BPT<long long, Order>` - 候补订单管理（使用哈希键）

**特性**：
- 支持同一键对应多个值
- 使用BPTCacheManager优化IO性能
- 支持分裂与合并操作维护树平衡
- 模板实例化集中管理，避免链接时冲突

### 4.2 内存缓存策略

为了提高性能，系统实现了多层缓存机制：

**1. LRU缓存**：
- 在`cache.hpp`中实现完整的LRU缓存系统
- 保留最近使用的数据页在内存中
- 支持脏页跟踪（dirty page tracking）
- 自动淘汰最久未使用的数据页

**2. BPTCacheManager**：
- 专门为B+树操作设计的缓存管理器
- 管理节点的读取和写入缓存
- 支持批量刷新脏页到磁盘
- 提供eviction callback机制

**3. MemoryRiver优化**：
- 实现`ensureFileOpen()`机制保持文件句柄打开
- 减少频繁的文件打开/关闭操作
- 支持移动语义，避免文件句柄冲突
- 提供flush机制确保关键数据及时写入

**缓存策略**：
```cpp
template <class Key, class Value>
class LRUCache {
  // 容量可配置，默认1024个条目
  size_t capacity_;
  // 支持脏页标记和批量写回
  void mark_dirty(const Key& key, bool is_dirty = true);
  sjtu::vector<Key> get_dirty_keys() const;
  // 淘汰回调，在页面被替换时调用
  void set_eviction_callback(EvictionCallback callback);
};
```

### 4.3 文件存储设计

系统文件组织结构：

```
/项目根目录
  ├── seat.memoryriver           # 座位管理MemoryRiver文件
  ├── users.index                # 用户B+树索引文件
  ├── users.block                # 用户B+树数据文件  
  ├── train.index                # 车次B+树索引文件
  ├── train.block                # 车次B+树数据文件
  ├── station.index              # 站点B+树索引文件
  ├── station.block              # 站点B+树数据文件
  ├── route.index                # 路线B+树索引文件
  ├── route.block                # 路线B+树数据文件
  ├── order.index                # 订单B+树索引文件
  ├── order.block                # 订单B+树数据文件
  ├── pending.index              # 候补订单B+树索引文件
  └── pending.block              # 候补订单B+树数据文件
```

**当前实现的存储机制**：

1. **MemoryRiver模式**：
   - 用于座位管理，直接二进制文件读写
   - 每个SeatMap占用固定大小空间
   - 支持随机访问和原地更新

2. **B+树存储模式**：
   - 用于用户、车次、站点管理
   - 索引文件(.index)存储内部节点信息和元数据
   - 数据文件(.block)存储叶子节点数据

**文件命名规则**：
- MemoryRiver文件：`{功能名}.memoryriver`
- B+树索引文件：`{功能名}.index`
- B+树数据文件：`{功能名}.block`

### 4.4 内存/磁盘交互策略

1. **延迟写入**：修改操作首先在内存中进行，定期批量写入磁盘
2. **预读取**：根据访问局部性，预读可能使用的数据块
3. **分块读写**：数据以块为单位进行IO操作，减少IO次数
4. **顺序写入**：新数据优先顺序写入，提高写入性能

## 5. 核心算法设计

### 5.1 命令解析算法

本系统采用“参数映射 + 分发器”组合方式进行命令解析，兼顾灵活性与健壮性：

1. **命令行分割**：首先将输入行分割为时间戳、命令名和参数部分。
2. **参数映射（ParamMap）**：遍历参数部分，识别所有以`-key value`形式出现的参数，存入自定义的`ParamMap`容器，支持无序、可选参数。
3. **命令分发（CommandSystem）**：根据命令名，通过基类指针的多态性，将参数映射传递给对应的命令处理器（CommandHandler），由其负责参数校验、类型转换和业务调用。

```cpp
// 命令注册 main.cpp
CommandSystem command_system;
UserManager user_manager;
command_system.registerHandler("login", new LoginHandler(user_manager));
...
// 命令的初步解析与分发 command/command_system.cpp
std::string parseAndExecute(const std::string& cmd_line,
                              std::string& timestamp);
// 各命令的二次解析与转交控制层
class LoginHandler : public CommandHandler {
 private:
  UserManager& user_manager;

 public:
  LoginHandler(UserManager& manager);
  std::string execute(const ParamMap& params, const std::string& timestamp) override;
};
...
```

**设计优点**：

- 支持命令参数无序、可选、灵活扩展
- 解析逻辑与业务逻辑解耦，便于维护和测试
- 参数校验、类型转换集中在分发器/处理器中，易于统一管理
- 便于后续扩展新命令或参数类型


### 5.2 车票查询算法

```
查询从站点S到站点T在日期D的车票：
1. 通过站点索引找出经过S的车次集合A：
   使用Hash::hashKey(S)在station_db中查找
2. 通过站点索引找出经过T的车次集合B：
   使用Hash::hashKey(T)在station_db中查找
3. 求交集C = A ∩ B，筛选同时经过S和T的车次
4. 对每个车次train_id in C：
   a. 在train_db中查询车次详细信息
   b. 检查train中S站在T站之前（站点索引顺序）
   c. 检查日期D是否在[sale_date_start, sale_date_end]内
   d. 计算出发到达时间：
      - 出发时间 = D + departure_times[S站索引]
      - 到达时间 = D + arrival_times[T站索引]（可能跨日）
   e. 计算票价：sum(prices[S站索引:T站索引))
   f. 查询座位余量：在seat_db中查找UniTrain{train_id, D}
5. 根据时间或价格排序筛选后的结果
```

### 5.3 换乘查询算法

换乘查询算法实现了复杂的多车次路径搜索，支持时间和价格优化：

```
查询从S到T在日期D的换乘方案：

1. 初始化阶段：
   a. 查询经过起点S的所有车次：train_manager.queryStation(start_station)
   b. 查询经过终点T的所有车次：train_manager.queryStation(end_station)
   c. 为每个终点车次建立有效换乘站点映射：
      stations_valid_from_end[i][station] = station_index
      （只包含终点站T之前的站点）

2. 双重嵌套搜索：
   for each train_from_start in trains_from_start:
     a. 验证车次在指定日期的有效性：
        检查 sale_date_start + departure_time <= D <= sale_date_end + departure_time
     b. 计算起点站的原始发车日期：origin_date1 = D - departure_times[start_index].hour/24
     c. 计算起点出发时间：start_time = TimePoint(origin_date1, departure_times[start_index])
     
     for each intermediate_station_i (start_index+1 to station_num-1):
       d. 计算到达中转站时间：arrival_time = TimePoint(origin_date1, arrival_times[i])
       
       for each train_to_end in trains_to_end:
         e. 跳过同一车次：if (train_to_end.train_id == train_from_start.train_id) continue
         f. 检查中转站有效性：
            if (stations_valid_from_end[j].find(intermediate_station) == end()) continue
         g. 获取中转站在终点车次中的索引：end_transfer_index

3. 复杂的跨日时间计算：
   a. 计算第二车次的销售日期范围对应时间点：
      sale_date_start_timepoint = TimePoint(sale_date_start, departure_times[transfer_index])
      sale_date_end_timepoint = TimePoint(sale_date_end, departure_times[transfer_index])
   b. 检查换乘时间可行性：if (sale_date_end_timepoint < arrival_time) continue
   c. 计算第二车次的原始发车日期：
      if (sale_date_start_timepoint >= arrival_time):
          origin_date2 = sale_date_start
      else:
          transfer_departure_point = Time(departure_times[transfer_index].hour % 24, minute)
          if (arrival_time.time <= transfer_departure_point):
              origin_date2 = arrival_time.date - departure_times[transfer_index].hour/24
          else:
              origin_date2 = arrival_time.date - departure_times[transfer_index].hour/24 + 1

4. 成本和时间计算：
   a. 计算总旅行时间：travel_time = arrive_at_end_station - start_time
   b. 计算总价格：travel_price = (end_train.prices[end_index] - end_train.prices[transfer_index]) + 
                                 (start_train.prices[transfer_index] - start_train.prices[start_index])

5. 多级优化比较：
   时间优先模式（is_time == true）：
   time_minor = travel_time < min_time ||
                (travel_time == min_time && travel_price < min_price) ||
                (travel_time == min_time && travel_price == min_price && train_id作为第三关键字)
   
   价格优先模式（is_time == false）：
   price_minor = travel_price < min_price ||
                 (travel_price == min_price && travel_time < min_time) ||
                 (travel_price == min_price && travel_time == min_time && train_id作为第三关键字)

6. 座位可用性查询：
   a. 查询第一车次座位图：seat_manager.querySeat(seat_map_pos_1, ticket1.origin_date - sale_date_1)
   b. 查询第二车次座位图：seat_manager.querySeat(seat_map_pos_2, ticket2.origin_date - sale_date_2)
   c. 计算各段座位余量：
      ticket1.seats = seat_map1.queryAvailableSeat(start_index, transfer_index)
      ticket2.seats = seat_map2.queryAvailableSeat(transfer_index, end_index)

7. 结果输出：
   if (无有效换乘方案): 输出 "0"
   else: 输出两行TicketInfo格式的车票信息
```

**算法特点**：
- **O(n × m × k)复杂度**：n为起点车次数，m为终点车次数，k为平均站点数
- **精确的跨日计算**：处理车次跨越多日的复杂时间逻辑
- **多级排序优化**：支持时间/价格优先的多关键字比较
- **座位集成查询**：换乘路径确定后实时查询座位可用性
- **边界条件处理**：销售日期验证、同车次过滤、换乘时间可行性检查

### 5.4 座位预订算法

```
座位预订处理（当前实现）：
1. 用户购票请求：train_id, date, start_station, end_station, num
2. 根据车次和日期计算座位图位置：
   seat_map_pos = start_pos + date_from_sale_start * sizeof(SeatMap)
3. 从seat_db中读取对应的SeatMap：
   seat_db.read(seat_map, seat_map_pos)
4. 检查座位可用性：
   调用seat_map.isSeatAvailable(start_station, end_station, seat)
   遍历区间[start_station, end_station)，确保每段都有足够座位
5. 预订座位：
   调用seat_map.bookSeat(start_station, end_station, seat)
   对区间内每个站点进行座位扣减：seat_num[i] -= seat
6. 更新存储：
   seat_db.update(seat_map, seat_map_pos)

算法特点：
- 直接文件访问，通过位置偏移快速定位SeatMap
- 连续存储模式，同一车次不同日期的座位图顺序存放
- O(k)时间复杂度检查和更新区间（k为区间长度）
- 支持原地更新，避免频繁的文件重写
- 座位操作逻辑封装在SeatMap结构中，代码模块化清晰
```

### 5.5 哈希函数设计

系统实现了支持中文字符的哈希函数：

```cpp
namespace Hash {
    long long hashKey(const std::string& key);
    long long hashKey(const FixedString<N>& key);
}
```

**特点**：
- 支持中文字符串的稳定哈希
- 分布均匀，减少哈希冲突
- 用于将字符串键转换为B+树的long long键

### 5.6 FixedString优化

为减少内存碎片和提高缓存效率，系统使用FixedString：

```cpp
template<int N>
class FixedString {
private:
    char data[N];
    
public:
    FixedString();
    FixedString(const std::string& str);
    FixedString(const char* str);
    
    std::string toString() const;
    // 比较运算符 <, >, ==, !=, <=, >=
};
```

**优势**：
- 固定内存布局，避免动态分配
- 支持直接序列化到文件
- 减少内存碎片，提高缓存命中率

### 5.7 订单处理算法

#### 5.7.1 购票流程

```
购票处理流程（buy_ticket命令）：
1. 参数验证：
   a. 检查用户是否已登录：isLoggedIn(username)
   b. 验证车次是否存在且已发布：queryTrain(train_id)
   c. 验证出发站和到达站在车次路线中的顺序
   d. 验证日期是否在销售范围内：[sale_date_start, sale_date_end]

2. 座位可用性检查：
   a. 计算座位图位置：seat_map_pos = start_pos + date_offset * sizeof(SeatMap)
   b. 读取座位图：seat_manager.getSeatMap(train_id, date)
   c. 检查区间座位：seat_map.checkSeatAvailability(from_idx, to_idx, num)

3. 订单处理分支：
   if (座位充足) {
       a. 立即扣减座位：seat_map.bookSeat(from_idx, to_idx, num)
       b. 创建成功订单：Order(..., SUCCESS, timestamp)
       c. 存储订单：order_manager.addOrder(order)
       d. 返回成功信息：车次、时间、价格等
   } else if (用户选择候补) {
       a. 创建候补订单：Order(..., PENDING, timestamp)
       b. 加入候补队列：order_manager.addPendingOrder(order)
       c. 返回候补确认：queue
   } else {
       返回座位不足错误：-1
   }
```

#### 5.7.2 退票流程

```
退票处理流程（refund_ticket命令）：
1. 订单查询与验证：
   a. 查询用户所有订单：order_manager.queryOrder(username)
   b. 根据时间戳定位目标订单
   c. 验证订单状态（只能退SUCCESS状态的订单）

2. 座位释放：
   a. 解析订单中的车次、日期、区间信息
   b. 释放座位：seat_manager.releaseSeat(train_id, date, from_idx, to_idx, num)
   c. 更新座位图到磁盘

3. 候补订单处理：
   a. 查询该车次日期的候补队列：queryPendingOrder(train_id, date)
   b. 按时间戳顺序处理候补订单：
      for each pending_order in queue:
          if (座位足够处理此候补订单) {
              i. 扣减相应座位
              ii. 更新订单状态：PENDING → SUCCESS
              iii. 从候补队列移除：removeFromPending()
              iv. 更新到正式订单存储
          }

4. 原订单状态更新：
   a. 更新订单状态：SUCCESS → REFUNDED
   b. 返回退票成功确认
```

#### 5.7.3 订单查询算法

```
订单查询流程（query_order命令）：
1. 用户验证：
   a. 检查用户是否已登录：isLoggedIn(username)

2. 订单检索：
   a. 使用用户名作为键查询：order_manager.queryOrder(username)
   b. B+树查找：order_db.find(username)返回用户所有订单

3. 结果排序与格式化：
   a. 按时间戳排序（升序）：sort by timestamp
   b. 格式化输出：[timestamp] train_id from to date time price num status
   c. 状态显示：SUCCESS, PENDING, REFUNDED
```

#### 5.7.4 候补队列管理

```
候补队列的关键设计：
1. 存储结构：
   - 键：Hash::hashKey(train_id + date.toString())
   - 值：Order对象（包含完整订单信息）
   - 排序：按timestamp自动排序（B+树特性）

2. 队列处理时机：
   - 退票时自动处理：refund_ticket触发
   - 按FIFO原则：最早提交的候补订单优先处理
   - 原子性保证：一个候补订单的处理要么完全成功要么完全失败

3. 座位分配策略：
   - 贪心算法：优先满足时间戳最小的订单
   - 部分满足：如果座位不足以满足某个候补订单，跳过处理下一个
   - 连续处理：直到队列为空或剩余座位无法满足任何候补订单
```

**算法复杂度分析**：

- **购票查询**: O(log n) - B+树查找车次信息
- **座位检查**: O(k) - k为出发到到达站的区间长度  
- **订单存储**: O(log n) - B+树插入操作
- **退票处理**: O(log n + m) - n为订单数量，m为候补队列长度
- **候补队列处理**: O(m × k) - m个候补订单，每个需要O(k)时间检查座位

## 6. 接口设计

### 6.1 命令行接口

系统通过标准输入输出进行交互，命令格式为：
`[<timestamp>] <cmd> -<key_1> <argument_1> -<key_2> <argument_2> ...`

### 6.2 模块间接口

#### 用户管理接口

```cpp
class UserManager {
private:
    BPT<uint64_t, User> user_db;
    sjtu::map<std::string, int> logged_in_users{};  // from username to privilege
    bool is_first_user{false};

public:
    UserManager();

    int addUser(const std::string& cur_username, const std::string& username,
                const std::string& password, const std::string& name,
                const std::string& mail_addr, const int& privilege);

    int login(const std::string& username, const std::string& password);

    int logout(const std::string& username);

    sjtu::pair<int, UserProfile> queryProfile(const std::string& cur_username,
                                              const std::string& username);

    sjtu::pair<int, UserProfile> modifyProfile(const std::string& cur_username,
                                               const std::string& username,
                                               const std::string& password,
                                               const std::string& name,
                                               const std::string& mail_addr,
                                               const int& privilege);

    // 检查用户是否已登录，返回权限级别或-1
    int isLoggedIn(const std::string& username);
};
```

#### 车次管理接口

```cpp
class TrainManager {
private:
    BPT<FixedString<20>, Train> train_db;              // 车次信息存储
    BPT<FixedString<30>, FixedString<20>> station_db;  // 站点到车次映射
    BPT<Route, FixedString<20>> route_db;              // 路线到车次映射

public:
    TrainManager();
    
    // 添加车次，返回状态码
    int addTrain(const Train& train);
    
    // 删除车次，返回状态码
    int deleteTrain(const std::string& train_id);
    
    // 发布车次，返回状态码（引用返回车次对象供座位管理器使用）
    int releaseTrain(const std::string& train_id, Train& train);
    
    // 查询车次信息，通过引用返回车次对象
    int queryTrain(const std::string& train_id, Train& train);
    int queryTrain(const FixedString<20>& train_id, Train& train);
    
    // 更新车次信息
    void updateTrain(const Train& train);
    
    // 查询经过指定站点的车次
    sjtu::vector<FixedString<20>> queryStation(const std::string& station_id);
    sjtu::vector<FixedString<20>> queryStation(const FixedString<30>& station_id);
    
    // 查询指定路线的车次
    sjtu::vector<FixedString<20>> queryRoute(const Route& route);
};
```

#### 座位管理接口

```cpp
class SeatManager {
private:
    MemoryRiver<SeatMap> seat_db;  // 座位信息存储（使用MemoryRiver进行高效文件访问）

public:
    SeatManager();
    
    // 初始化座位图（发布车次时调用）
    void initSeat(const Train& train, int& train_seat);
    
    // 查询指定位置的座位分布
    SeatMap querySeat(int start_pos, int& seat_map_pos, int date_from_sale_start);
    
    // 预订座位
    int bookSeat(int seat_map_pos, int start_station, int end_station, 
                 int seat, SeatMap& seat_map);
    
    // 退票释放座位
    void releaseSeat(int seat_map_pos, int start_station, int end_station,
                     int seat, SeatMap& seat_map);
};
```

**实现特点**：
- 使用MemoryRiver进行直接文件I/O，避免复杂的B+树操作
- 连续存储同一车次不同日期的座位图，支持高效的日期偏移访问
- 座位操作直接在SeatMap结构上进行，支持O(k)复杂度的区间更新（k为区间长度）
- 支持原地更新，避免频繁的文件重写操作

#### 订单管理接口

```cpp
class OrderManager {
private:
    BPT<FixedString<20>, Order> order_db;    // 用户订单存储
    BPT<uint64_t, Order> pending_db;         // 候补订单存储

public:
    OrderManager();
    
    // 添加订单
    void addOrder(const Order& order);
    
    // 添加候补订单
    void addPendingOrder(const Order& order);
    
    // 更新订单状态
    void updateOrderStatus(const std::string& username, const Order& order,
                          OrderStatus status);
    void updateOrderStatus(const FixedString<20>& username, const Order& order,
                          OrderStatus status);
    
    // 从候补队列移除订单
    void removeFromPending(const FixedString<20>& train_id, const Date& date,
                          const Order& order);
    
    // 查询用户订单
    sjtu::vector<Order> queryOrder(const std::string& username);
    
    // 查询候补订单
    sjtu::vector<Order> queryPendingOrder(const FixedString<20>& train_id,
                                         const Date& date);
};
```
    void addOrder(const Order& order);
    
    // 添加候补订单
    void addPendingOrder(const Order& order);
    
    // 查询用户订单
    sjtu::vector<Order> queryOrder(const std::string& username);
    
    // 更新订单状态
    void updateOrderStatus(const std::string& username, const Order& order, OrderStatus status);
    void updateOrderStatus(const FixedString<20>& username, const Order& order, OrderStatus status);
    
    // 从候补队列移除订单
    void removeFromPending(const FixedString<20>& unitrain, const Date& date, const Order& order);
    
    // 查询候补订单
    sjtu::vector<Order> queryPendingOrder(const FixedString<20>& train_id, const Date& date);
};
```

#### 命令处理系统接口

系统采用命令模式设计，通过统一的命令处理器接口处理所有用户请求：

```cpp
class CommandHandler {
public:
    virtual void execute(const ParamMap& params, const std::string& timestamp) = 0;
    virtual ~CommandHandler() = default;
};

class CommandSystem {
public:
    void registerHandler(const std::string& command, CommandHandler* handler);
    void processCommand(const std::string& line);
};
```

**主要命令处理器包括**：

- **用户管理命令**：`LoginHandler`, `AddUserHandler`, `LogoutHandler`, `QueryProfileHandler`, `ModifyProfileHandler`
- **车次管理命令**：`AddTrainHandler`, `DeleteTrainHandler`, `ReleaseTrainHandler`, `QueryTrainHandler`
- **票务查询命令**：`QueryTicketHandler`, `QueryTransferHandler`
- **订票管理命令**：`BuyTicketHandler`, `RefundTicketHandler`, `QueryOrderHandler`

每个处理器负责解析参数、调用相应的管理器方法并格式化输出结果。

#### 命令处理器接口

**用户命令处理器**
```cpp
class LoginHandler : public CommandHandler {
private:
    UserManager& user_manager;
public:
    LoginHandler(UserManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class AddUserHandler : public CommandHandler {
private:
    UserManager& user_manager;
public:
    AddUserHandler(UserManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class LogoutHandler : public CommandHandler {
private:
    UserManager& user_manager;
public:
    LogoutHandler(UserManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryProfileHandler : public CommandHandler {
private:
    UserManager& user_manager;
public:
    QueryProfileHandler(UserManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class ModifyProfileHandler : public CommandHandler {
private:
    UserManager& user_manager;
public:
    ModifyProfileHandler(UserManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};
```

**车次命令处理器**
```cpp
class AddTrainHandler : public CommandHandler {
private:
    TrainManager& train_manager;
public:
    AddTrainHandler(TrainManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class DeleteTrainHandler : public CommandHandler {
private:
    TrainManager& train_manager;
public:
    DeleteTrainHandler(TrainManager& manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class ReleaseTrainHandler : public CommandHandler {
private:
    TrainManager& train_manager;
    SeatManager& seat_manager;
public:
    ReleaseTrainHandler(TrainManager& manager, SeatManager& seat_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryTrainHandler : public CommandHandler {
private:
    TrainManager& train_manager;
    SeatManager& seat_manager;
public:
    QueryTrainHandler(TrainManager& train_manager, SeatManager& seat_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryTransferHandler : public CommandHandler {
private:
    TrainManager& train_manager;
    SeatManager& seat_manager;
public:
    QueryTransferHandler(TrainManager& train_manager, SeatManager& seat_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};
```

**订单命令处理器**
```cpp
class QueryTicketHandler : public CommandHandler {
private:
    TrainManager& train_manager;
    SeatManager& seat_manager;
public:
    QueryTicketHandler(TrainManager& train_manager, SeatManager& seat_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class BuyTicketHandler : public CommandHandler {
private:
    TrainManager& train_manager;
    SeatManager& seat_manager;
    UserManager& user_manager;
    OrderManager& order_manager;
public:
    BuyTicketHandler(TrainManager& train_manager, SeatManager& seat_manager,
                     UserManager& user_manager, OrderManager& order_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class QueryOrderHandler : public CommandHandler {
private:
    OrderManager& order_manager;
    UserManager& user_manager;
public:
    QueryOrderHandler(OrderManager& order_manager, UserManager& user_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};

class RefundTicketHandler : public CommandHandler {
private:
    OrderManager& order_manager;
    UserManager& user_manager;
    TrainManager& train_manager;
    SeatManager& seat_manager;
public:
    RefundTicketHandler(OrderManager& order_manager, UserManager& user_manager,
                        TrainManager& train_manager, SeatManager& seat_manager);
    void execute(const ParamMap& params, const std::string& timestamp) override;
};
```

#### 票务信息结构

```cpp
// 单张票务信息
struct TicketInfo {
    FixedString<20> train_id{};
    FixedString<30> from{};
    FixedString<30> to{};
    TimePoint start_time{};
    TimePoint end_time{};
    Date origin_date{};
    int price{};
    int seats{};
    int minutes{};

    TicketInfo() = default;
    TicketInfo(const FixedString<20>& train_id, const FixedString<30>& from,
               const FixedString<30>& to, const TimePoint& start_time,
               const TimePoint& end_time, const Date& origin_date, int price,
               int seats);

    std::string format() const;
};

// 换乘票务信息结构
struct TransferTicketInfo {
    TicketInfo first_ticket{};   // 第一段车票信息
    TicketInfo second_ticket{};  // 第二段车票信息

    std::string format() const {
        return first_ticket.format() + '\n' + second_ticket.format();
    }
};
```

**接口特点**：
- **一致的依赖注入**：所有命令处理器使用引用参数进行依赖注入，确保高效的内存访问
- **模块化设计**：每个命令处理器专注于特定功能，支持清晰的职责分离
- **统一执行接口**：所有处理器实现相同的execute接口，支持多态调用
- **实时数据集成**：与管理器模块深度集成，提供实时的数据查询和更新操作

## 7. 容错与异常处理

系统设计了完整的异常处理机制(项目第一阶段不实现)：

1. **输入验证**：严格验证用户输入的参数格式和合法性
2. **权限检查**：所有操作进行权限验证，防止非法访问
3. **事务完整性**：关键操作保证原子性，防止数据不一致
4. **文件操作保护**：文件操作使用异常处理，保证安全
5. **索引一致性**：保持内存数据和文件数据的一致性

### 自定义异常类

```cpp
// 基于stl/exceptions.hpp
class Exception : public std::exception {
private:
    std::string message;
    
public:
    Exception(const std::string& msg) : message(msg) {}
    
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class FileException : public Exception {
public:
    FileException(const std::string& msg) : Exception("File error: " + msg) {}
};

class InvalidOperationException : public Exception {
public:
    InvalidOperationException(const std::string& msg) 
        : Exception("Invalid operation: " + msg) {}
};
```

## 8. 性能优化策略

为应对内存限制和高效查询需求，系统采取以下优化策略：

1. **索引优化**：设计适合查询模式的索引结构
2. **缓存命中率优化**：针对热点数据设计多级缓存
3. **批处理操作**：合并多次IO为批量IO
4. **延迟写入**：非关键数据延迟写入磁盘
5. **预读取机制**：利用空间局部性预先读取相关数据
6. **分布式扩展**：支持数据和请求的分布式处理
7. **内存池技术**：减少内存分配开销
8. **命令优先级处理**：根据指令常用度设计优化策略

## 9. 测试策略

系统测试分为以下几个层次：

1. **单元测试**：测试各个组件的基本功能
2. **模块测试**：测试各个模块的完整功能
3. **集成测试**：测试模块间的协作
4. **系统测试**：针对完整系统功能
5. **压力测试**：测试系统在高负载下的表现
6. **内存测试**：确保系统在内存限制下正常工作
7. **多次启动测试**：验证数据持久化功能

## 10. 项目组织与实现计划

### 10.1 目录结构

```
/
├── src/                      # 源代码目录
│   ├── command/              # 命令处理模块
│   │   ├── command_system.cpp/.hpp    # 命令系统核心
│   │   ├── user_command.cpp/.hpp      # 用户相关命令处理器
│   │   ├── train_command.cpp/.hpp     # 车次相关命令处理器
│   │   ├── order_command.cpp/.hpp     # 订单相关命令处理器
│   │   └── system_command.cpp/.hpp    # 系统命令处理器
│   ├── controller/           # 业务逻辑控制层
│   │   ├── user_manager.cpp/.hpp      # 用户管理器
│   │   ├── train_manager.cpp/.hpp     # 车次管理器
│   │   ├── seat_manager.cpp/.hpp      # 座位管理器
│   │   └── order_manager.cpp/.hpp     # 订单管理器
│   ├── model/                # 数据模型层
│   │   ├── user.hpp          # 用户数据结构
│   │   ├── train.hpp         # 车次数据结构
│   │   ├── seat.hpp          # 座位数据结构
│   │   ├── order.hpp         # 订单数据结构
│   │   ├── ticket.hpp        # 票务信息结构
│   │   ├── time.hpp          # 时间相关结构
│   │   └── station.hpp       # 站点相关结构
│   ├── storage/              # 存储引擎层
│   │   ├── bplus_tree.cpp/.hpp        # B+树索引实现
│   │   ├── memory_river.hpp           # 内存河流文件访问
│   │   ├── cache.hpp                  # 缓存管理系统
│   │   └── index_block.hpp            # 索引块管理
│   ├── utilities/            # 工具函数模块
│   │   ├── hash.hpp          # 哈希函数（支持中文）
│   │   ├── limited_sized_string.hpp  # 固定大小字符串
│   │   ├── merge_sort.hpp             # 归并排序算法
│   │   └── parse_by_char.hpp          # 字符解析工具
│   ├── stl/                  # STL替代实现
│   │   ├── vector.hpp        # 动态数组
│   │   ├── map.hpp           # 映射容器
│   │   ├── list.hpp          # 链表容器
│   │   ├── priority_queue.hpp # 优先队列
│   │   ├── hash_map.hpp      # 哈希映射
│   │   ├── utility.hpp       # 实用工具（pair等）
│   │   └── exceptions.hpp    # 异常处理
│   └── main.cpp              # 主程序入口
├── docs/                     # 项目文档
│   ├── overall-design-document.md  # 总体设计文档
│   └── acquirement.md              # 需求文档
├── testcases/                # 测试用例
│   ├── *.in                  # 输入测试文件
│   ├── *.out                 # 期望输出文件
│   ├── config.*.js/.yml/.json # 测试配置文件
│   └── extract.py            # 测试工具脚本
├── build/                    # 构建输出目录
├── code                      # 可执行文件
├── CMakeLists.txt           # CMake构建脚本
├── clean.sh                 # 清理脚本
├── run-test                 # 测试运行脚本
└── README.md                # 项目说明文档
```

### 10.2 实现计划

#### 第一阶段（已完成）：基础设施建设
✅ **完成状态**：已完成核心基础设施
- **STL替代容器**：实现vector, map, list, priority_queue, hash_map等
- **B+树索引结构**：完整的模板化B+树实现，支持文件持久化
- **内存/文件管理系统**：MemoryRiver高效文件访问，缓存管理
- **基础数据结构**：FixedString, 哈希函数（支持中文），时间处理系统
- **异常处理框架**：自定义异常类体系

#### 第二阶段（已完成）：核心业务功能
✅ **完成状态**：核心模块全部实现
- **命令解析系统**：CommandSystem完整框架，ParamMap参数解析
- **用户管理模块**：用户注册/登录/登出/查询/修改，权限验证
- **车次管理模块**：车次添加/删除/发布/查询，站点索引系统
- **座位管理模块**：SeatManager完整实现，支持座位预订/释放
- **订单管理模块**：订单创建/查询/退票，候补队列处理

#### 第三阶段（已完成）：高级查询功能
✅ **完成状态**：复杂查询算法已实现
- **票务查询系统**：支持时间/价格排序的直达车票查询
- **换乘查询算法**：O(n×m×k)复杂度的全局换乘路径搜索
- **候补订单系统**：自动候补队列处理，FIFO优先级管理
- **座位集成查询**：实时余票信息，跨日期时间计算

#### 第四阶段（进行中）：系统优化与完善
🔄 **当前状态**：持续优化中
- **性能优化**：查询缓存机制，内存管理优化
- **测试完善**：单元测试，集成测试，压力测试覆盖
- **边界条件处理**：异常情况处理，数据一致性保证
- **代码重构**：代码质量提升，文档完善

#### 第五阶段（计划中）：GUI图形界面开发
⏳ **预计时间**：下一阶段实现
- **GUI框架选择**：评估Qt/GTK+/wxWidgets等跨平台GUI框架
- **界面设计**：用户友好的图形界面设计
- **前端适配**：将现有命令行接口适配为GUI接口
- **用户体验优化**：交互流程优化，视觉设计改进

## 11. GUI扩展设计（下一阶段规划）

> **注意**：GUI功能当前尚未实现，以下为下一阶段的设计规划。

### 11.1 设计理念

为便于未来扩展图形用户界面，当前系统已采用了有利于GUI扩展的架构设计：

### 11.1 模型-视图分离

本项目采用严格的模型-视图-控制器(MVC)模式设计：

```
+---------------+    +---------------+    +---------------+
|     视图      |    |    控制器     |    |     模型      |
|   (View)      |<-->| (Controller)  |<-->|    (Model)    |
+---------------+    +---------------+    +---------------+
     CLI/GUI界面       业务逻辑层         数据模型层
```

- **模型层**：包含核心数据结构和业务逻辑（已实现）
- **控制器层**：处理用户输入并调用业务逻辑（已实现）
- **视图层**：当前为CLI界面，未来扩展为GUI界面

这种分离确保命令行版本的所有业务逻辑可直接被GUI版本复用。

### 11.2 为GUI扩展预留的架构特性

当前系统已采用的有利于GUI扩展的设计模式：

#### 11.2.1 结构化数据返回

当前业务方法已返回结构化数据对象而非格式化字符串：

```cpp
// 已实现的结构化数据类型
struct TicketInfo {
    FixedString<20> train_id{};
    FixedString<30> from{};
    FixedString<30> to{};
    TimePoint start_time{};
    TimePoint end_time{};
    Date origin_date{};
    int price{};
    int seats{};
    int minutes{};
    
    std::string format() const;  // CLI格式化方法
};

struct TransferTicketInfo {
    TicketInfo first_ticket{};
    TicketInfo second_ticket{};
    
    std::string format() const;  // CLI格式化方法
};

// 业务管理器返回结构化数据
class TrainManager {
public:
    sjtu::vector<FixedString<20>> queryStation(const std::string& station_id);
    // 其他方法...
};
```

#### 11.2.2 命令处理器模式

当前采用的命令处理器模式便于适配GUI事件：

```cpp
// 已实现的命令处理器基类
class CommandHandler {
public:
    virtual void execute(const ParamMap& params, const std::string& timestamp) = 0;
    virtual ~CommandHandler() = default;
};

// GUI扩展时可改为返回结构化结果
template<typename ResultType>
class GUICommandHandler {
public:
    virtual ResultType execute(const ParamMap& params) = 0;
    virtual ~GUICommandHandler() = default;
};
```

### 11.3 事件与观察者模式

实现观察者模式以支持GUI事件处理：

```cpp
// 事件监听器接口（未来GUI扩展时实现）
class StateChangeListener {
public:
    virtual void onStateChanged(const std::string& stateType, void* data) = 0;
    virtual ~StateChangeListener() {}
};

// 系统状态管理（未来GUI扩展时实现）
class SystemState {
private:
    sjtu::vector<StateChangeListener*> listeners;
public:
    void addListener(StateChangeListener* listener);
    void notifyStateChanged(const std::string& stateType, void* data);
};
```

### 11.4 异步操作支持

设计异步操作机制，防止GUI界面在长时间操作中阻塞：

```cpp
// 异步操作结果回调（未来GUI扩展时实现）
template<typename T>
using Callback = std::function<void(T)>;

class AsyncTicketService {
public:
    void queryTicketAsync(const std::string& from, const std::string& to,
                       const std::string& date, const std::string& sortType,
                       Callback<TicketInfo> callback);
                       
    void buyTicketAsync(const std::string& username, const std::string& trainID,
                     const std::string& date, int num, const std::string& from,
                     const std::string& to, bool waitlist,
                     Callback<Order> callback);
};
```

### 11.5 GUI界面规划

> **未来规划**：以下为下一阶段GUI开发的界面设计规划，当前尚未实现。

未来GUI实现将包括以下主要界面：

#### 11.5.1 主界面设计
- **顶部菜单栏**：文件、用户、车次、订单等功能菜单
- **左侧导航栏**：用户登录状态、快速功能入口
- **中央工作区**：动态切换不同功能模块的界面
- **底部状态栏**：系统状态、操作提示信息

#### 11.5.2 核心功能界面
- **用户管理界面**：登录/注册表单，用户信息展示和编辑
- **车票查询界面**：出发地/目的地选择，日期选择，结果表格展示
- **换乘查询界面**：多段行程规划，路径可视化展示
- **订单管理界面**：订单列表，状态跟踪，退票操作
- **车次管理界面**：管理员专用，车次信息管理

#### 11.5.3 用户体验特性
- **响应式设计**：适配不同屏幕尺寸
- **实时更新**：座位余量、订单状态的实时刷新
- **操作反馈**：加载提示、操作成功/失败消息
- **数据可视化**：车次路线图、座位分布图

## 12. 项目状态总结

使用结构化错误返回而非简单的成功/失败码：（待实现）

```cpp
struct OperationResult {
    bool success;
    int errorCode;
    std::string errorMessage;
    
    // 构造函数
    static OperationResult success() { return {true, 0, ""}; }
    static OperationResult error(int code, const std::string& msg) {
        return {false, code, msg};
    }
};

// 附加操作具体结果
template<typename T>
struct Result : public OperationResult {
    T data;  // 操作成功时的返回数据
    
    static Result<T> success(const T& value) {
        Result<T> result;
        result.success = true;
        result.errorCode = 0;
        result.data = value;
        return result;
    }
    
    static Result<T> error(int code, const std::string& msg) {
        Result<T> result;
        result.success = false;
        result.errorCode = code;
        result.errorMessage = msg;
        return result;
    }
};
```

### 11.6 GUI规划

未来的GUI界面将包含以下主要页面：

1. **登录/注册页面**：用户身份验证
2. **主页/控制台**：系统功能导航
3. **用户信息管理**：个人资料查看与修改
4. **车票查询页面**：
   - 输入查询条件（出发地、目的地、日期）
   - 结果展示与排序
   - 换乘查询选项
5. **车票订购流程**：
   - 选择座位
   - 确认订单
   - 支付模拟
6. **订单管理**：
   - 订单列表展示
   - 订单详情查看
   - 退票操作
7. **管理员功能**（权限控制）：
   - 车次管理
   - 用户管理
   - 系统状态监控

通过以上架构设计，系统已为未来GUI扩展奠定了良好基础，可以在保持核心业务逻辑不变的前提下，轻松扩展为图形界面应用。

## 12. 项目状态总结

### 12.1 已完成模块

✅ **基础设施层**
- B+树存储引擎（支持模板化键值对）
- 缓存管理系统
- 文件I/O层
- 自定义STL容器（vector, map, list等）
- FixedString内存优化字符串
- 哈希函数（支持中文）

✅ **数据模型层**  
- User数据结构及序列化
- Train数据结构（支持到达/出发时间）
- Date/Time/TimePoint时间处理系统
- UniTrain复合键设计
- SeatMap直接座位管理

✅ **命令解析系统**
- ParamMap参数映射
- CommandSystem命令分发器
- CommandHandler基类框架
- 健壮的命令行解析

✅ **用户管理模块**
- 用户注册、登录、登出
- 用户信息查询和修改
- 权限验证机制
- 密码安全处理

✅ **车次管理模块**
- 车次添加、删除、发布
- 车次信息查询
- 站点到车次的映射索引

✅ **座位管理模块**
- SeatManager完整实现（使用MemoryRiver）
- 座位初始化、查询、预订、释放功能
- 连续存储模式，支持日期偏移访问
- 直接文件I/O，高效的原地更新
- SeatMap结构封装座位操作逻辑

✅ **票务查询模块**
- QueryTicketHandler完整实现
- 支持时间和价格排序
- 站点索引查询和过滤逻辑
- 座位余量查询集成
- TicketInfo结构化数据返回

✅ **订单管理模块**
- OrderManager完整实现（使用B+树）
- 订单创建、查询、状态更新功能
- BuyTicketHandler支持购票和候补队列
- QueryOrderHandler支持订单查询
- RefundTicketHandler支持退票和候补处理
- 候补队列自动处理机制

✅ **换乘查询模块**
- QueryTransferHandler完整实现
- 复杂换乘路径搜索算法（O(n × m × k)复杂度）
- 跨日期时间计算和验证逻辑
- 多级比较排序（时间/价格优先+车次ID tie-breaker）
- 与座位管理系统深度集成的余票查询
- TransferTicketInfo结构化数据封装
- 边界条件处理和异常安全保证

### 12.2 待实现模块

⏳ **性能优化**
- 查询缓存机制
- 内存管理优化
- 批量I/O优化

⏳ **完整测试体系**
- 性能测试套件
- 压力测试
- 边界条件测试

### 12.3 技术债务

- [ ] 异常处理机制需要完善
- [ ] 日志系统需要添加
- [ ] QueryTicket性能瓶颈需要进一步优化

## 13. 总结

本火车票订票系统设计文档详细描述了系统的整体架构、核心数据结构、关键算法和实现细节。

### 13.1 系统特色

1. **高效存储**：使用B+树索引和直接座位管理，实现高效的数据存储和查询
2. **内存优化**：FixedString和缓存管理确保在有限内存下的高性能
3. **模块化设计**：清晰的分层架构，便于维护和扩展
4. **中文支持**：哈希函数和字符串处理支持中文车站名
5. **时间精确**：完整的日期时间系统，支持跨日计算
6. **扩展性强**：命令系统设计支持快速添加新功能

### 13.2 技术亮点

- **参数映射+分发器**命令解析模式，健壮且灵活
- **直接座位管理**，O(k)复杂度的区间更新
- **MemoryRiver优化**，高效的文件I/O和缓存管理
- **模板化B+树**，支持多种数据类型的统一存储
- **缓存管理**，平衡内存使用和IO性能

### 13.3 后续发展

系统为未来扩展提供了良好基础：
- GUI界面可基于当前MVC架构快速开发
- 网络功能可通过增加网络层实现
- 分布式扩展可基于现有数据分片设计
- 性能优化可通过调整缓存策略和索引结构实现

本文档将随着项目开发进度持续更新，确保设计与实现的一致性。