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
4. **票务管理模块**：处理票务相关操作，包括查询车票、购票、候补等
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
};

// 时间相关数据结构
struct Date {
    int month, day;
    std::string toString() const;
    Date operator+(int days) const;
};

struct Time {
    int hour, minute;
    std::string toString() const;
    Time operator+(int minutes) const;
    Time operator+(const Time& other) const;
};

struct TimePoint {
    Date date;
    Time time;
    std::string toString() const;
    TimePoint operator+(const Time& other) const;
    TimePoint operator+(int minutes) const;
    // 比较运算符 <, >, ==, !=, <=, >=
};

// 车次+日期的复合键，用于座位管理
struct UniTrain {
    FixedString<20> train_id;
    Date date;
    // 比较运算符 <, >, ==, !=, <=, >=
};
```

#### 3.2.3 座位管理数据结构

```cpp
struct SeatMap {
    int total_seats;                      // 总座位数
    int station_num;                      // 站点数量
    int seat_num[MAX_STATION_NUM];        // 各区间剩余座位数量
    
    // 检查指定区间是否有足够座位
    bool is_seat_available(int start_station, int end_station, int seat);
    
    // 预订座位，直接减少各区间的座位数
    bool book_seat(int start_station, int end_station, int seat);
};
```

#### 3.2.4 订单数据结构

```cpp
enum OrderStatus { SUCCESS, PENDING, REFUNDED };

struct Order {
    int id;                    // 订单ID
    std::string username;      // 用户名
    std::string trainID;       // 车次ID
    std::string from;          // 出发站
    std::string to;            // 到达站
    std::string date;          // 乘车日期
    std::string leavingTime;   // 出发时间
    std::string arrivingTime;  // 到达时间
    int price;                 // 总票价
    int num;                   // 购票数量
    OrderStatus status;        // 订单状态
    int timestamp;             // 创建时间戳
};
```

### 3.3 索引结构

系统使用B+树实现关键索引，核心索引包括：

1. **用户索引**：`BPT<long long, User>` - 使用用户名哈希值作为键
2. **车次索引**：`BPT<long long, Train>` - 使用车次ID哈希值作为键
3. **站点索引**：`BPT<long long, FixedString<20>>` - 站点名哈希值 -> 车次ID列表
4. **座位索引**：`BPT<UniTrain, SeatMap>` - 车次+日期复合键 -> 座位分布图
5. **订单索引**：(username, timestamp) -> Order (待实现)
6. **候补队列索引**：(trainID, date) -> PriorityQueue<Order> (待实现)

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

**实例化的B+树类型**：
- `BPT<long long, User>` - 用户管理
- `BPT<long long, Train>` - 车次管理  
- `BPT<long long, FixedString<20>>` - 站点到车次映射
- `BPT<UniTrain, SeatMap>` - 座位管理

**特性**：
- 支持同一键对应多个值
- 使用缓存管理器优化IO性能
- 支持分裂与合并操作维护树平衡

### 4.2 内存缓存策略

为了提高性能，系统实现了缓存：

1. **LRU缓存**：保留最近使用的数据页在内存中
2. **脏页跟踪**：追踪修改过的数据页，定期或在系统关闭时写回磁盘

### 4.3 文件存储设计

系统文件组织结构：

```
/data
  ├── user_data.dat            # 用户B+树数据文件
  ├── user_index.dat           # 用户B+树索引文件
  ├── train_data.dat           # 车次B+树数据文件
  ├── train_index.dat          # 车次B+树索引文件
  ├── station_data.dat         # 站点B+树数据文件
  ├── station_index.dat        # 站点B+树索引文件
  ├── seat_data.dat            # 座位B+树数据文件
  ├── seat_index.dat           # 座位B+树索引文件
  ├── orders_data.dat          # 订单B+树数据文件 (待实现)
  ├── orders_index.dat         # 订单B+树索引文件 (待实现)
  └── pending_orders_data.dat  # 候补订单数据文件 (待实现)
```

**文件命名规则**：
- 每个B+树实例对应两个文件：数据文件(_data.dat)和索引文件(_index.dat)
- 数据文件存储叶子节点和内部节点
- 索引文件存储树的元信息（根节点地址、高度等）

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
  std::string execute(const ParamMap& params) override;
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

```
查询从S到T的换乘方案：
1. 找出所有经过S的车次集合A：
   使用Hash::hashKey(S)在station_db中查找
2. 找出所有经过T的车次集合B：
   使用Hash::hashKey(T)在station_db中查找
3. 对于A中每个车次train_a：
   a. 找出train_a经过的所有中间站M（S之后的站点）
   b. 对于B中每个车次train_b：
      i. 检查train_b是否经过M（T之前的站点）
      ii. 计算train_a到达M的时间：date + arrival_times[M在train_a中的索引]
      iii. 计算train_b从M出发的时间：date + departure_times[M在train_b中的索引]
      iv. 检查换乘时间是否充足（到达时间 < 出发时间）
      v. 计算总时间、总价格
4. 选择最优解(根据排序策略：时间优先或价格优先)
```

### 5.4 座位预订算法

```
座位预订处理：
1. 用户购票请求：train_id, date, start_station, end_station, num
2. 构造查询键：UniTrain{train_id, date}
3. 在seat_db中查找对应的SeatMap
4. 检查区间[start_station, end_station)的座位余量：
   a. 遍历该区间内的每个i，找到最小剩余座位数min_seats
   b. 如果min_seats >= num，则可以预订
5. 预订成功时进行座位扣减：
   a. 对于区间[start_station, end_station)中的每个i
   b. seat_num[i] -= num （减少该区间的剩余座位）
6. 将更新后的SeatMap写回seat_db

算法特点：
- 直接存储各区间剩余座位数，逻辑简单清晰
- O(k)时间复杂度检查和更新区间（k为区间长度）
- 支持单座位级别的精确管理
- 便于座位余量的快速查询
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

## 6. 接口设计

### 6.1 命令行接口

系统通过标准输入输出进行交互，命令格式为：
`[<timestamp>] <cmd> -<key_1> <argument_1> -<key_2> <argument_2> ...`

### 6.2 模块间接口

#### 用户管理接口

```cpp
class UserManager {
public:
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
};
```

#### 车次管理接口

```cpp
class TrainManager {
private:
    BPT<long long, Train> train_db;              // 车次信息存储
    BPT<long long, FixedString<20>> station_db;  // 站点到车次映射

public:
    TrainManager();
    
    // 添加车次，返回状态码
    int addTrain(const Train& train);
    
    // 删除车次，返回状态码
    int deleteTrain(const std::string& train_id);
    
    // 发布车次，返回状态码
    int releaseTrain(const std::string& train_id);
    
    // 查询车次信息，通过引用返回车次对象
    int queryTrain(const std::string& train_id, Train& train);
};
```

#### 座位管理接口

```cpp
class SeatManager {
private:
    BPT<UniTrain, SeatMap> seat_db;  // 座位信息存储

public:
    SeatManager();
    
    // 查询指定车次和日期的座位分布
    SeatMap querySeat(const UniTrain& unitrain);
    
    // 预订座位（待实现）
    // bool bookSeat(const UniTrain& unitrain, int start_station, 
    //               int end_station, int num);
    
    // 退票释放座位（待实现）
    // bool refundSeat(const UniTrain& unitrain, int start_station,
    //                 int end_station, int num);
};
```

#### 订单管理接口

```cpp
class OrderManager {
public:
    // 查询订单
    std::string queryOrder(const std::string& username);
    
    // 退票
    bool refundTicket(const std::string& username, int orderIndex);
    
    // 处理候补队列
    void processPendingOrders(const std::string& trainID, const std::string& date);
};
```

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
6. **内存池技术**：减少内存分配开销
7. **命令优先级处理**：根据指令常用度设计优化策略

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
├── src/
│   ├── stl/           # STL替代实现
│   ├── storage/       # 存储引擎实现
│   ├── command/       # 命令解析与处理
│   ├── user/          # 用户管理模块
│   ├── train/         # 车次管理模块
│   ├── ticket/        # 票务管理模块
│   ├── order/         # 订单管理模块
│   └── main.cpp       # 主程序
├── docs/              # 文档
├── testcases/         # 测试用例
└── CMakeLists.txt         # 构建脚本
```

### 10.2 实现计划

1. **第一阶段**：实现基础设施
   - STL替代容器
   - B+树索引结构
   - 内存/文件管理系统

2. **第二阶段**：实现核心功能
   - 命令解析系统
   - 用户管理模块
   - 车次管理模块

3. **第三阶段**：实现高级功能
   - 票务查询与购买
   - 换乘查询算法
   - 候补订单系统

4. **第四阶段**：优化与测试
   - 性能优化
   - 内存管理优化
   - 全面测试

## 11. GUI扩展设计

为便于未来扩展图形用户界面(GUI)，系统在设计时采取了以下策略：

### 11.1 模型-视图分离

本项目采用严格的模型-视图-控制器(MVC)模式设计：

```
+---------------+    +---------------+    +---------------+
|     视图      |    |    控制器     |    |     模型      |
|   (View)      |<-->| (Controller)  |<-->|    (Model)    |
+---------------+    +---------------+    +---------------+
      GUI界面           业务逻辑           数据与状态
```

- **模型层**：包含核心数据结构和业务逻辑
- **控制器**：处理用户输入并更新模型
- **视图层**：负责数据可视化展示

这种分离确保命令行版本的所有业务逻辑可直接被GUI版本复用。

### 11.2 结构化数据返回

所有业务方法返回结构化数据而非格式化字符串：

```cpp
// 不使用格式化字符串作为返回值
struct TicketQueryResult {
    int totalCount;
    struct TicketInfo {
        std::string trainID;
        std::string from;
        std::string to;
        std::string departureTime;
        std::string arrivalTime;
        int price;
        int availableSeats;
    };
    sjtu::vector<TicketInfo> tickets;
};

// 业务方法返回结构化数据
class TicketService {
public:
    TicketQueryResult queryTicket(const std::string& from, const std::string& to,
                               const std::string& date, const std::string& sortType);
};

// 当前CLI版本格式化为字符串
class CommandHandler {
private:
    TicketService service;
    
    std::string formatTicketQueryResult(const TicketQueryResult& result) {
        // 格式化逻辑...
    }
};
```

### 11.3 事件与观察者模式

实现观察者模式以支持GUI事件处理：

```cpp
// 事件监听器接口
class StateChangeListener {
public:
    virtual void onStateChanged(const std::string& stateType, void* data) = 0;
    virtual ~StateChangeListener() {}
};

// 系统状态管理
class SystemState {
private:
    sjtu::vector<StateChangeListener*> listeners;
public:
    void addListener(StateChangeListener* listener);
    void removeListener(StateChangeListener* listener);
    void notifyStateChanged(const std::string& stateType, void* data);
};
```

### 11.4 异步操作支持

设计异步操作机制，防止GUI界面在长时间操作中阻塞：

```cpp
// 异步操作结果回调
template<typename T>
using Callback = std::function<void(T)>;

class AsyncTicketService {
public:
    void queryTicketAsync(const std::string& from, const std::string& to,
                       const std::string& date, const std::string& sortType,
                       Callback<TicketQueryResult> callback);
                       
    void buyTicketAsync(const std::string& username, const std::string& trainID,
                     const std::string& date, int num, const std::string& from,
                     const std::string& to, bool waitlist,
                     Callback<OrderResult> callback);
};
```

### 11.5 详细错误信息

使用结构化错误返回而非简单的成功/失败码：

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

### 11.7 设计适配

将现有命令行接口适配为GUI接口：

```cpp
// 命令行接口
class CommandLineInterface {
private:
    void executeCommand(const std::string& cmd, const ParamMap& params);
};

// GUI适配器
class GUIAdapter {
private:
    UserService userService;
    TrainService trainService;
    TicketService ticketService;
    OrderService orderService;
    
public:
    // GUI调用的方法
    Result<User> login(const std::string& username, const std::string& password);
    Result<sjtu::vector<TicketInfo>> searchTickets(const SearchCriteria& criteria);
    Result<Order> purchaseTicket(const TicketPurchaseInfo& info);
    // ...其他方法
};
```

通过以上设计，系统可以在保持核心业务逻辑不变的前提下，轻松扩展为GUI应用。

## 12. 当前实现状态

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

✅ **座位管理基础**
- SeatManager基础框架
- 座位查询接口

### 12.2 进行中模块

🔄 **票务查询模块**
- query_ticket基础算法（需完善）
- 站点索引查询逻辑
- 时间和价格计算

🔄 **订单管理模块**
- 订单数据结构设计中
- 购票流程开发中

### 12.3 待实现模块

⏳ **高级票务功能**
- 换乘查询算法
- 候补购票系统
- 退票处理

⏳ **性能优化**
- 查询缓存机制
- 内存管理优化
- 批量I/O优化

⏳ **完整测试体系**
- 性能测试套件
- 压力测试
- 边界条件测试

### 12.4 技术债务

- [ ] B+树模板实例化需要统一管理
- [ ] 异常处理机制需要完善
- [ ] 日志系统需要添加
- [ ] 内存泄漏检测和修复

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
- **复合键设计**，UniTrain有效组合车次和日期
- **模板化B+树**，支持多种数据类型的统一存储
- **缓存管理**，平衡内存使用和IO性能

### 13.3 后续发展

系统为未来扩展提供了良好基础：
- GUI界面可基于当前MVC架构快速开发
- 网络功能可通过增加网络层实现
- 分布式扩展可基于现有数据分片设计
- 性能优化可通过调整缓存策略和索引结构实现

本文档将随着项目开发进度持续更新，确保设计与实现的一致性。