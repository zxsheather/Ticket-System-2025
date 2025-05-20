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

基于项目目录结构，我们实现了以下容器：

- **vector**：动态数组，支持随机访问和动态扩容
- **list**：双向链表，支持高效插入和删除
- **map**：有序映射，基于AVL树实现
- **hash_map**：无序映射，基于哈希表实现
- **priority_queue**：优先队列，用于候补订单系统
- **utility**：提供pair等工具类型
- **FixedString**：固定长度字符串，避免内存碎片化

### 3.2 业务数据结构

#### 3.2.1 用户数据结构

```cpp
// 用户核心数据结构
struct User {
    FixedString<USERNAME_MAX_LEN> username;  // 用户名，唯一标识符
    FixedString<PASSWORD_MAX_LEN> password;  // 密码
    FixedString<NAME_MAX_LEN> name;          // 用户真实姓名
    FixedString<MAIL_MAX_LEN> mail_addr;     // 邮箱地址
    int privilege;                          // 用户权限级别(0-10)
    
    // 构造函数
    User(const std::string& _username, const std::string& _password, 
         const std::string& _name, const std::string& _mail_addr, const int& _privilege);
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
    std::string trainID;          // 车次唯一标识符
    int stationNum;               // 车站数量
    std::string stations[100];    // 各站名称
    int seatNum;                  // 座位数量
    int prices[99];               // 相邻站点间票价
    std::string startTime;        // 发车时间
    int travelTimes[99];          // 相邻站点间行车时间
    int stopoverTimes[98];        // 各站停留时间
    std::string saleDate[2];      // 售票日期区间
    char type;                    // 列车类型
    bool released;                // 是否已发布
};
```

#### 3.2.3 车票数据结构

```cpp
struct Ticket {
    std::string trainID;          // 车次ID
    std::string date;             // 车票日期
    int seats[100][100];          // 各区间余票数量
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

1. **用户索引**：username -> User
2. **车次索引**：trainID -> Train
3. **站点索引**：station -> List<trainID>
4. **订单索引**：(username, timestamp) -> Order
5. **候补队列索引**：(trainID, date) -> PriorityQueue<Order>

## 4. 存储系统设计

### 4.1 B+树索引

B+树是系统的核心索引结构，提供以下接口：

```cpp
    void insert(const Key &key, const Value &value); //插入一个键值对
    void remove(const Key &key, const Value &value); //删除一个键值对
    sjtu::vector<Value> find(const Key &key); //查找key下所有的值
    bool empty(); //判断树是否为空
    bool exists(const Key &key); //判断key是否存在
```
注意： 在本项目中B+树同一个键可以对应多个值。

### 4.2 内存缓存策略

为了提高性能，系统实现了缓存：

1. **LRU缓存**：保留最近使用的数据页在内存中
2. **脏页跟踪**：追踪修改过的数据页，定期或在系统关闭时写回磁盘

### 4.3 文件存储设计

系统文件组织结构：

```
/data
  ├── users.dat             # 用户数据文件
  ├── users_index.dat       # 用户索引文件
  ├── trains.dat            # 车次基本信息
  ├── trains_index.dat      # 车次索引文件
  ├── station_index.dat     # 站点索引文件
  ├── seats/               
  │   ├── <trainID>_<date>.dat  # 各日期车次的座位信息
  │   └── ...
  ├── orders.dat            # 订单信息
  ├── orders_index.dat      # 订单索引
  └── pending_orders.dat    # 候补订单信息
```

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
1. 通过站点索引找出经过S的车次集合A
2. 通过站点索引找出经过T的车次集合B
3. 求交集C = A ∩ B
4. 对每个车次c in C：
   a. 检查c中S站在T站之前
   b. 检查日期D是否在售票日期内
   c. 计算从S到T的时间、价格和余票
5. 根据时间或价格排序筛选后的结果
```

### 5.3 换乘查询算法

```
查询从S到T的换乘方案：
1. 找出所有经过S的车次集合A
2. 找出所有经过T的车次集合B
3. 对于A中每个车次a：
   a. 找出a经过的所有中间站M
   b. 对于B中每个车次b：
      i. 检查b是否经过M
      ii. 检查a到达M的时间是否早于b从M出发的时间
      iii. 计算总时间、总价格
4. 选择最优解(根据排序策略)
```

### 5.4 候补购票算法

```
候补购票处理：
1. 用户购票请求余票不足时，检查是否允许候补
2. 如果允许，创建候补订单加入候补队列
3. 有退票发生时：
   a. 增加座位余量
   b. 遍历候补队列中的订单，按时间戳排序
   c. 对于每个候补订单，检查是否可满足(余票>=订单票数)
   d. 满足的订单从候补队列移除，标记为成功
```

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
public:
    // 添加车次
    bool addTrain(const Train& train);
    
    // 删除车次
    bool deleteTrain(const std::string& trainID);
    
    // 发布车次
    bool releaseTrain(const std::string& trainID);
    
    // 查询车次信息
    std::string queryTrain(const std::string& trainID, const std::string& date);
};
```

#### 票务管理接口

```cpp
class TicketManager {
public:
    // 查询车票
    std::string queryTicket(const std::string& from, const std::string& to, 
                           const std::string& date, const std::string& sortType);
    
    // 查询换乘
    std::string queryTransfer(const std::string& from, const std::string& to,
                             const std::string& date, const std::string& sortType);
    
    // 购买车票
    std::string buyTicket(const std::string& username, const std::string& trainID, 
                         const std::string& date, int num, const std::string& from, 
                         const std::string& to, bool waitlist);
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
└── Makefile           # 构建脚本
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