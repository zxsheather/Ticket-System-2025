const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const winston = require('winston');

// 配置日志
const logger = winston.createLogger({
  level: 'info',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.errors({ stack: true }),
    winston.format.json()
  ),
  defaultMeta: { service: 'ticket-controller' },
  transports: [
    new winston.transports.File({ filename: 'logs/ticket-controller.log' }),
    new winston.transports.Console({
      format: winston.format.simple()
    })
  ]
});

class TicketController {
  constructor() {
    this.backendProcess = null;
    this.isRunning = false;
    this.mockMode = false;
    this.commandQueue = [];
    this.responseCallbacks = {};
    this.timestampCounter = 1;
    
    // C++后端程序路径
    this.backendPath = path.join(__dirname, '../../code');
    
    this.startBackend();
  }
  
  startBackend() {
    try {
      logger.info('启动C++后端进程...');
      
      // 检查后端程序是否存在
      if (!fs.existsSync(this.backendPath)) {
        logger.warn(`C++后端程序不存在: ${this.backendPath}`);
        logger.info('启用模拟模式，Web前端将独立运行');
        this.isRunning = false;
        this.mockMode = true;
        return;
      }
      
      this.backendProcess = spawn(this.backendPath, [], {
        stdio: ['pipe', 'pipe', 'pipe'],
        cwd: path.dirname(this.backendPath)
      });
      
      this.isRunning = true;
      this.mockMode = false;
      
      // 处理后端输出
      this.backendProcess.stdout.on('data', (data) => {
        this.handleBackendResponse(data.toString());
      });
      
      // 处理后端错误
      this.backendProcess.stderr.on('data', (data) => {
        logger.error(`C++后端错误: ${data.toString()}`);
      });
      
      // 处理进程退出
      this.backendProcess.on('close', (code) => {
        logger.warn(`C++后端进程退出，代码: ${code}`);
        this.isRunning = false;
        
        // 如果不是正常退出，尝试重启
        if (code !== 0) {
          setTimeout(() => {
            this.startBackend();
          }, 5000);
        }
      });
      
      logger.info('C++后端进程启动成功');
      
    } catch (error) {
      logger.error('启动C++后端进程失败:', error);
      logger.info('启用模拟模式，Web前端将独立运行');
      this.isRunning = false;
      this.mockMode = true;
    }
  }
  
  handleBackendResponse(data) {
    const lines = data.trim().split('\n');
    
    lines.forEach(line => {
      if (line.trim()) {
        logger.debug(`后端响应: ${line}`);
        
        // 解析时间戳和响应内容
        const match = line.match(/^\[(\d+)\]\s*(.*)$/);
        if (match) {
          const timestamp = match[1];
          const response = match[2];
          
          if (this.responseCallbacks[timestamp]) {
            const callback = this.responseCallbacks[timestamp];
            
            if (callback.multiline) {
              // 多行响应：收集所有内容
              callback.response = response;
              callback.lines = [response];
              
              // 设置定时器，在短时间内收集完整响应
              setTimeout(() => {
                if (this.responseCallbacks[timestamp]) {
                  const fullResponse = callback.lines.join('\n');
                  callback.callback(fullResponse);
                  delete this.responseCallbacks[timestamp];
                }
              }, 100);
            } else {
              // 单行响应：立即返回
              callback.callback(response);
              delete this.responseCallbacks[timestamp];
            }
          }
        } else {
          // 处理多行响应的后续行
          for (const timestamp in this.responseCallbacks) {
            const callback = this.responseCallbacks[timestamp];
            if (callback.multiline && callback.lines) {
              callback.lines.push(line);
              break;
            }
          }
        }
      }
    });
  }
  
  executeCommand(command, params = {}) {
    return new Promise((resolve, reject) => {
      // 模拟模式处理
      if (this.mockMode) {
        logger.debug(`模拟模式执行命令: ${command}`, params);
        const mockResponse = this.generateMockResponse(command, params);
        setTimeout(() => resolve(mockResponse), 100); // 模拟网络延迟
        return;
      }
      
      if (!this.isRunning || !this.backendProcess) {
        reject(new Error('C++后端未运行'));
        return;
      }
      
      const timestamp = this.timestampCounter++;
      let commandLine = `[${timestamp}] ${command}`;
      
      // 添加参数
      for (const [key, value] of Object.entries(params)) {
        if (value !== undefined && value !== null) {
          commandLine += ` -${key} ${value}`;
        }
      }
      
      commandLine += '\n';
      
      logger.debug(`发送命令: ${commandLine.trim()}`);
      
      // 设置响应回调
      this.responseCallbacks[timestamp] = {
        callback: (response) => resolve(response),
        multiline: command === 'query_train' || command === 'query_ticket' || command === 'query_transfer' || command === 'query_order',
        response: '',
        timestamp: timestamp
      };
      
      // 设置超时
      setTimeout(() => {
        if (this.responseCallbacks[timestamp]) {
          delete this.responseCallbacks[timestamp];
          reject(new Error('命令执行超时'));
        }
      }, 30000); // 30秒超时
      
      // 发送命令
      try {
        this.backendProcess.stdin.write(commandLine);
      } catch (error) {
        delete this.responseCallbacks[timestamp];
        reject(error);
      }
    });
  }
  
  // 用户管理命令
  async login(username, password) {
    try {
      const response = await this.executeCommand('login', {
        u: username,
        p: password
      });
      // login 成功返回 0，失败返回 -1
      return { success: response === '0', response, privilege: response === '0' ? 1 : 0 };
    } catch (error) {
      logger.error('登录失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async logout(username) {
    try {
      const response = await this.executeCommand('logout', {
        u: username
      });
      return { success: response === '0', response };
    } catch (error) {
      logger.error('登出失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async addUser(currentUser, username, password, name, mailAddr, privilege) {
    try {
      const response = await this.executeCommand('add_user', {
        c: currentUser,
        u: username,
        p: password,
        n: name,
        m: mailAddr,
        g: privilege
      });
      return { success: response === '0', response };
    } catch (error) {
      logger.error('添加用户失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async queryProfile(currentUser, username) {
    try {
      const response = await this.executeCommand('query_profile', {
        c: currentUser,
        u: username
      });
      
      if (response === '-1') {
        return { success: false, response };
      }
      
      // 解析用户信息
      const parts = response.split(' ');
      return {
        success: true,
        profile: {
          username: parts[0],
          name: parts[1],
          mailAddr: parts[2],
          privilege: parseInt(parts[3])
        }
      };
    } catch (error) {
      logger.error('查询用户信息失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async modifyProfile(currentUser, username, password, name, mailAddr, privilege) {
    try {
      const params = { c: currentUser, u: username };
      if (password) params.p = password;
      if (name) params.n = name;
      if (mailAddr) params.m = mailAddr;
      if (privilege !== undefined) params.g = privilege;
      
      const response = await this.executeCommand('modify_profile', params);
      
      if (response === '-1') {
        return { success: false, response };
      }
      
      // 解析修改后的用户信息
      const parts = response.split(' ');
      return {
        success: true,
        profile: {
          username: parts[0],
          name: parts[1],
          mailAddr: parts[2],
          privilege: parseInt(parts[3])
        }
      };
    } catch (error) {
      logger.error('修改用户信息失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  // 车次管理命令
  async addTrain(trainId, stationNum, seatNum, stations, prices, startTime, travelTimes, stopoverTimes, saleDate, type) {
    try {
      const response = await this.executeCommand('add_train', {
        i: trainId,
        n: stationNum,
        m: seatNum,
        s: stations.join('|'),
        p: prices.join('|'),
        x: startTime,
        t: travelTimes.join('|'),
        o: stopoverTimes.join('|'),
        d: saleDate.join('|'),
        y: type
      });
      return { success: response === '0', response };
    } catch (error) {
      logger.error('添加车次失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async deleteTrain(trainId) {
    try {
      const response = await this.executeCommand('delete_train', {
        i: trainId
      });
      return { success: response === '0', response };
    } catch (error) {
      logger.error('删除车次失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async releaseTrain(trainId) {
    try {
      const response = await this.executeCommand('release_train', {
        i: trainId
      });
      return { success: response === '0', response };
    } catch (error) {
      logger.error('发布车次失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async queryTrain(trainId, date) {
    try {
      const response = await this.executeCommand('query_train', {
        i: trainId,
        d: date
      });
      
      if (response === '-1') {
        return { success: false, response };
      }
      
      return { success: true, trainInfo: response };
    } catch (error) {
      logger.error('查询车次失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  // 票务查询命令
  async queryTicket(from, to, date, sortBy = 'time') {
    try {
      const response = await this.executeCommand('query_ticket', {
        s: from,
        t: to,
        d: date,
        p: sortBy
      });
      
      const lines = response.split('\n');
      const count = parseInt(lines[0]);
      
      if (count === 0) {
        return { success: true, tickets: [] };
      }
      
      const tickets = [];
      for (let i = 1; i <= count; i++) {
        if (lines[i]) {
          tickets.push(this.parseTicketInfo(lines[i]));
        }
      }
      
      return { success: true, tickets };
    } catch (error) {
      logger.error('查询车票失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async queryTransfer(from, to, date, sortBy = 'time') {
    try {
      const response = await this.executeCommand('query_transfer', {
        s: from,
        t: to,
        d: date,
        p: sortBy
      });
      
      if (response === '0') {
        return { success: true, transfer: null };
      }
      
      const lines = response.split('\n');
      if (lines.length >= 2) {
        return {
          success: true,
          transfer: {
            firstTrain: this.parseTicketInfo(lines[0]),
            secondTrain: this.parseTicketInfo(lines[1])
          }
        };
      }
      
      return { success: false, response };
    } catch (error) {
      logger.error('查询换乘失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  // 订单管理命令
  async buyTicket(username, trainId, date, num, from, to, queue = false) {
    try {
      const params = {
        u: username,
        i: trainId,
        d: date,
        n: num,
        f: from,
        t: to
      };
      
      if (queue) {
        params.q = 'true';
      }
      
      const response = await this.executeCommand('buy_ticket', params);
      
      if (response === '-1') {
        return { success: false, response };
      } else if (response === 'queue') {
        return { success: true, queued: true, response };
      } else {
        return { success: true, totalPrice: parseInt(response), response };
      }
    } catch (error) {
      logger.error('购票失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async queryOrder(username) {
    try {
      const response = await this.executeCommand('query_order', {
        u: username
      });
      
      if (response === '-1') {
        return { success: false, response };
      }
      
      const lines = response.split('\n');
      const count = parseInt(lines[0]);
      const orders = [];
      
      for (let i = 1; i <= count; i++) {
        if (lines[i]) {
          orders.push(this.parseOrderInfo(lines[i]));
        }
      }
      
      return { success: true, orders };
    } catch (error) {
      logger.error('查询订单失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async refundTicket(username, orderNum = 1) {
    try {
      const response = await this.executeCommand('refund_ticket', {
        u: username,
        n: orderNum
      });
      return { success: response === '0', response };
    } catch (error) {
      logger.error('退票失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  // 解析票务信息
  parseTicketInfo(ticketLine) {
    // 格式: <trainID> <FROM> <LEAVING_TIME> -> <TO> <ARRIVING_TIME> <PRICE> <SEAT>
    const parts = ticketLine.split(' ');
    if (parts.length >= 8) {
      return {
        trainId: parts[0],
        from: parts[1],
        leavingTime: parts[2] + ' ' + parts[3], // 日期 时间
        to: parts[5],
        arrivingTime: parts[6] + ' ' + parts[7], // 日期 时间
        price: parseInt(parts[8]),
        seats: parseInt(parts[9])
      };
    }
    // 兼容旧格式
    return {
      trainId: parts[0],
      from: parts[1],
      leavingTime: parts[2],
      to: parts[4],
      arrivingTime: parts[5],
      price: parseInt(parts[6]),
      seats: parseInt(parts[7])
    };
  }
  
  // 解析订单信息
  parseOrderInfo(orderLine) {
    // 格式: [<STATUS>] <trainID> <FROM> <LEAVING_TIME> -> <TO> <ARRIVING_TIME> <PRICE> <NUM>
    const statusMatch = orderLine.match(/^\[(success|pending|refunded)\]/);
    if (statusMatch) {
      const status = statusMatch[1];
      const remaining = orderLine.substring(statusMatch[0].length).trim();
      const parts = remaining.split(' ');
      
      if (parts.length >= 8) {
        return {
          status: status,
          trainId: parts[0],
          from: parts[1],
          leavingTime: parts[2] + ' ' + parts[3], // 日期 时间
          to: parts[5],
          arrivingTime: parts[6] + ' ' + parts[7], // 日期 时间
          price: parseInt(parts[8]),
          num: parseInt(parts[9])
        };
      }
    }
    
    // 兼容旧格式
    const parts = orderLine.split(' ');
    return {
      status: parts[0] === '[success]' ? 'success' : parts[0] === '[pending]' ? 'pending' : 'refunded',
      trainId: parts[1],
      from: parts[2],
      leavingTime: parts[3],
      to: parts[5],
      arrivingTime: parts[6],
      price: parseInt(parts[7]),
      num: parseInt(parts[8])
    };
  }
  
  // 系统管理命令
  async clean() {
    try {
      const response = await this.executeCommand('clean');
      return { success: response === '0', response };
    } catch (error) {
      logger.error('清理数据失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  async exit() {
    try {
      const response = await this.executeCommand('exit');
      return { success: response === 'bye', response };
    } catch (error) {
      logger.error('退出系统失败:', error);
      return { success: false, error: error.message };
    }
  }
  
  // 生成模拟响应
  generateMockResponse(command, params = {}) {
    logger.info(`生成模拟响应: ${command}`, params);
    
    // 生成模拟时间戳
    const timestamp = this.timestampCounter - 1; // 使用刚才用过的时间戳
    
    switch (command) {
      case 'login':
        // 登录成功返回 0，失败返回 -1
        return params.u === 'admin' && params.p === 'wrongpass' ? '-1' : '0';
        
      case 'add_user':
        if (params.u === 'admin') {
          return '-1'; // 用户已存在  
        }
        return '0'; // 注册成功
        
      case 'query_profile':
        return `${params.u || 'testuser'} TestUser test@example.com 1`;
        
      case 'modify_profile':
        return `${params.u || 'testuser'} ${params.n || 'TestUser'} ${params.m || 'test@example.com'} ${params.g || 1}`;
        
      case 'logout':
        return '0'; // 退出成功
        
      case 'query_ticket':
        // 生成模拟车票数据
        const mockTickets = [
          'G123 Beijing 07-01 08:00 -> Shanghai 07-01 14:30 450 50',
          'D456 Beijing 07-01 10:15 -> Shanghai 07-01 16:45 280 80',
          'G789 Beijing 07-01 12:30 -> Shanghai 07-01 18:00 450 30'
        ];
        return '3\n' + mockTickets.join('\n');
        
      case 'query_transfer':
        // 生成模拟中转数据
        return 'G123 Beijing 07-01 08:00 -> Nanjing 07-01 12:00 300 20\nD789 Nanjing 07-01 13:00 -> Shanghai 07-01 15:30 180 40';
        
      case 'buy_ticket':
        // 根据购买数量决定是否成功
        const ticketNum = parseInt(params.n) || 1;
        const pricePerTicket = 450; // 模拟票价
        if (ticketNum <= 50) {
          return (ticketNum * pricePerTicket).toString(); // 返回总价
        } else {
          return 'queue'; // 候补
        }
        
      case 'query_order':
        // 生成模拟订单数据
        const mockOrders = [
          '[success] G123 Beijing 07-01 08:00 -> Shanghai 07-01 14:30 450 1',
          '[pending] D456 Beijing 07-01 10:15 -> Shanghai 07-01 16:45 280 2'
        ];
        return '2\n' + mockOrders.join('\n');
        
      case 'refund_ticket':
        return '0'; // 退票成功
        
      case 'add_train':
        return '0'; // 添加列车成功
        
      case 'delete_train':
        return '0'; // 删除列车成功
        
      case 'release_train':
        return '0'; // 发布列车成功
        
      case 'query_train':
        // 生成模拟列车信息
        const trainInfo = [
          'G123 G',
          '上院 xx-xx xx:xx -> 07-01 19:19 0 1000',
          '中院 07-02 05:19 -> 07-02 05:24 114 1000',
          '下院 07-02 15:24 -> xx-xx xx:xx 628 x'
        ];
        return trainInfo.join('\n');
        
      case 'clean':
        return '0'; // 清理成功
        
      case 'exit':
        return 'bye'; // 退出
        
      default:
        logger.warn(`未知的模拟命令: ${command}`);
        return '0'; // 默认成功响应
    }
  }
  
  // 清理资源
  cleanup() {
    if (this.backendProcess && this.isRunning) {
      logger.info('正在关闭C++后端进程...');
      this.backendProcess.stdin.write('exit\n');
      
      setTimeout(() => {
        if (this.backendProcess && !this.backendProcess.killed) {
          this.backendProcess.kill('SIGTERM');
        }
      }, 5000);
    }
  }
}

module.exports = new TicketController();
