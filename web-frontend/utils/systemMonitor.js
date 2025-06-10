/**
 * 系统监控工具
 * 监控系统状态、性能指标和资源使用情况
 */

const os = require('os');
const fs = require('fs').promises;
const path = require('path');
const logger = require('./logger');

class SystemMonitor {
  constructor() {
    this.startTime = Date.now();
    this.metrics = {
      requests: 0,
      errors: 0,
      logins: 0,
      tickets: 0
    };
    this.intervals = new Map();
  }

  /**
   * 获取系统基本信息
   */
  getSystemInfo() {
    const uptime = process.uptime();
    const systemUptime = os.uptime();
    
    return {
      // 系统信息
      platform: os.platform(),
      arch: os.arch(),
      hostname: os.hostname(),
      
      // 运行时间
      processUptime: this.formatUptime(uptime),
      systemUptime: this.formatUptime(systemUptime),
      
      // Node.js 版本
      nodeVersion: process.version,
      
      // 启动时间
      startTime: new Date(this.startTime).toISOString()
    };
  }

  /**
   * 获取内存使用情况
   */
  getMemoryUsage() {
    const memoryUsage = process.memoryUsage();
    const totalMemory = os.totalmem();
    const freeMemory = os.freemem();
    const usedMemory = totalMemory - freeMemory;

    return {
      // 进程内存使用（字节）
      process: {
        rss: memoryUsage.rss,
        heapTotal: memoryUsage.heapTotal,
        heapUsed: memoryUsage.heapUsed,
        external: memoryUsage.external,
        arrayBuffers: memoryUsage.arrayBuffers
      },
      
      // 系统内存使用（字节）
      system: {
        total: totalMemory,
        free: freeMemory,
        used: usedMemory,
        usagePercent: ((usedMemory / totalMemory) * 100).toFixed(2)
      },
      
      // 格式化的内存使用（易读格式）
      formatted: {
        processRSS: this.formatBytes(memoryUsage.rss),
        processHeapUsed: this.formatBytes(memoryUsage.heapUsed),
        systemTotal: this.formatBytes(totalMemory),
        systemUsed: this.formatBytes(usedMemory),
        systemFree: this.formatBytes(freeMemory)
      }
    };
  }

  /**
   * 获取CPU使用情况
   */
  getCPUUsage() {
    const cpus = os.cpus();
    const loadAvg = os.loadavg();

    // 计算CPU使用率
    let totalIdle = 0;
    let totalTick = 0;

    cpus.forEach(cpu => {
      for (let type in cpu.times) {
        totalTick += cpu.times[type];
      }
      totalIdle += cpu.times.idle;
    });

    const idle = totalIdle / cpus.length;
    const total = totalTick / cpus.length;
    const usage = 100 - ~~(100 * idle / total);

    return {
      usage: usage,
      count: cpus.length,
      model: cpus[0].model,
      speed: cpus[0].speed,
      loadAverage: {
        '1min': loadAvg[0].toFixed(2),
        '5min': loadAvg[1].toFixed(2),
        '15min': loadAvg[2].toFixed(2)
      }
    };
  }

  /**
   * 获取磁盘使用情况
   */
  async getDiskUsage() {
    try {
      const stats = await fs.stat(__dirname);
      const logDir = path.join(__dirname, '../logs');
      
      let logSize = 0;
      try {
        const logFiles = await fs.readdir(logDir);
        for (const file of logFiles) {
          if (file.endsWith('.log')) {
            const fileStat = await fs.stat(path.join(logDir, file));
            logSize += fileStat.size;
          }
        }
      } catch (err) {
        logger.warn('获取日志文件大小失败:', err.message);
      }

      return {
        logSize: logSize,
        logSizeFormatted: this.formatBytes(logSize)
      };
    } catch (err) {
      logger.error('获取磁盘使用情况失败:', err.message);
      return {
        logSize: 0,
        logSizeFormatted: '0 B'
      };
    }
  }

  /**
   * 获取网络信息
   */
  getNetworkInfo() {
    const interfaces = os.networkInterfaces();
    const networkList = [];

    for (const name in interfaces) {
      interfaces[name].forEach(iface => {
        if (!iface.internal) {
          networkList.push({
            name: name,
            address: iface.address,
            family: iface.family,
            mac: iface.mac
          });
        }
      });
    }

    return networkList;
  }

  /**
   * 更新业务指标
   */
  updateMetrics(type, increment = 1) {
    if (this.metrics.hasOwnProperty(type)) {
      this.metrics[type] += increment;
    }
  }

  /**
   * 获取业务指标
   */
  getMetrics() {
    return {
      ...this.metrics,
      uptime: process.uptime(),
      timestamp: Date.now()
    };
  }

  /**
   * 重置指标
   */
  resetMetrics() {
    for (let key in this.metrics) {
      this.metrics[key] = 0;
    }
  }

  /**
   * 开始监控
   */
  startMonitoring(io) {
    this.io = io;
    
    // 每10秒发送系统状态
    const statusInterval = setInterval(() => {
      this.broadcastSystemStatus();
    }, 10000);
    
    this.intervals.set('status', statusInterval);
    
    logger.info('系统监控已启动');
  }

  /**
   * 停止监控
   */
  stopMonitoring() {
    this.intervals.forEach((interval, name) => {
      clearInterval(interval);
      logger.info(`停止监控间隔: ${name}`);
    });
    this.intervals.clear();
    logger.info('系统监控已停止');
  }

  /**
   * 广播系统状态
   */
  async broadcastSystemStatus() {
    if (!this.io) return;

    try {
      const systemInfo = this.getSystemInfo();
      const memoryUsage = this.getMemoryUsage();
      const cpuUsage = this.getCPUUsage();
      const diskUsage = await this.getDiskUsage();
      const networkInfo = this.getNetworkInfo();
      const metrics = this.getMetrics();

      const status = {
        timestamp: Date.now(),
        system: systemInfo,
        memory: memoryUsage,
        cpu: cpuUsage,
        disk: diskUsage,
        network: networkInfo,
        metrics: metrics
      };

      // 发送到管理员房间
      this.io.to('admin').emit('system_status', status);
      
      // 发送基本状态到所有连接
      this.io.emit('basic_status', {
        uptime: systemInfo.processUptime,
        memory: memoryUsage.formatted.processRSS,
        cpu: cpuUsage.usage,
        timestamp: Date.now()
      });

    } catch (err) {
      logger.error('广播系统状态失败:', err.message);
    }
  }

  /**
   * 格式化字节数
   */
  formatBytes(bytes) {
    if (bytes === 0) return '0 B';

    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));

    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  }

  /**
   * 格式化运行时间
   */
  formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = Math.floor(seconds % 60);

    if (days > 0) {
      return `${days}天 ${hours}小时 ${minutes}分钟`;
    } else if (hours > 0) {
      return `${hours}小时 ${minutes}分钟`;
    } else if (minutes > 0) {
      return `${minutes}分钟 ${secs}秒`;
    } else {
      return `${secs}秒`;
    }
  }

  /**
   * 获取系统健康状态
   */
  async getHealthStatus() {
    const memory = this.getMemoryUsage();
    const cpu = this.getCPUUsage();
    
    const health = {
      status: 'healthy',
      checks: {
        memory: {
          status: memory.system.usagePercent < 85 ? 'healthy' : 'warning',
          value: memory.system.usagePercent + '%'
        },
        cpu: {
          status: cpu.usage < 80 ? 'healthy' : 'warning',
          value: cpu.usage + '%'
        },
        uptime: {
          status: 'healthy',
          value: this.formatUptime(process.uptime())
        }
      }
    };

    // 如果有警告状态，将整体状态设为警告
    if (Object.values(health.checks).some(check => check.status === 'warning')) {
      health.status = 'warning';
    }

    return health;
  }
}

module.exports = new SystemMonitor();
