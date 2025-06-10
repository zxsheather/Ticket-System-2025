// 输入验证中间件
const { body, validationResult } = require('express-validator');

// 用户登录验证
const validateLogin = [
  body('username')
    .trim()
    .isLength({ min: 1, max: 20 })
    .withMessage('用户名长度必须在1-20个字符之间')
    .matches(/^[a-zA-Z][a-zA-Z0-9_]*$/)
    .withMessage('用户名必须以字母开头，只能包含字母、数字和下划线'),
  
  body('password')
    .isLength({ min: 1, max: 30 })
    .withMessage('密码长度必须在1-30个字符之间')
];

// 用户注册验证
const validateRegister = [
  body('username')
    .trim()
    .isLength({ min: 1, max: 20 })
    .withMessage('用户名长度必须在1-20个字符之间')
    .matches(/^[a-zA-Z][a-zA-Z0-9_]*$/)
    .withMessage('用户名必须以字母开头，只能包含字母、数字和下划线'),
  
  body('password')
    .isLength({ min: 1, max: 30 })
    .withMessage('密码长度必须在1-30个字符之间'),
    
  body('confirmPassword')
    .custom((value, { req }) => {
      if (value !== req.body.password) {
        throw new Error('确认密码与密码不匹配');
      }
      return true;
    }),
  
  body('name')
    .trim()
    .isLength({ min: 1, max: 20 })
    .withMessage('姓名长度必须在1-20个字符之间'),
  
  body('email')
    .trim()
    .isLength({ min: 1, max: 30 })
    .withMessage('邮箱长度必须在1-30个字符之间')
    .isEmail()
    .withMessage('请输入有效的邮箱地址')
];

// 车票搜索验证
const validateTicketSearch = [
  body('from')
    .trim()
    .isLength({ min: 1, max: 10 })
    .withMessage('出发站名长度必须在1-10个字符之间'),
  
  body('to')
    .trim()
    .isLength({ min: 1, max: 10 })
    .withMessage('到达站名长度必须在1-10个字符之间'),
  
  body('date')
    .matches(/^\d{2}-\d{2}$/)
    .withMessage('日期格式必须为MM-DD')
    .custom((value) => {
      const [month, day] = value.split('-').map(Number);
      if (month < 1 || month > 12) {
        throw new Error('月份必须在01-12之间');
      }
      if (day < 1 || day > 31) {
        throw new Error('日期必须在01-31之间');
      }
      return true;
    }),
  
  body('sortBy')
    .optional()
    .isIn(['time', 'cost'])
    .withMessage('排序方式必须是time或cost')
];

// 购票验证
const validateBuyTicket = [
  body('trainId')
    .trim()
    .isLength({ min: 1, max: 20 })
    .withMessage('车次ID长度必须在1-20个字符之间'),
  
  body('date')
    .matches(/^\d{2}-\d{2}$/)
    .withMessage('日期格式必须为MM-DD'),
  
  body('from')
    .trim()
    .isLength({ min: 1, max: 10 })
    .withMessage('出发站名长度必须在1-10个字符之间'),
  
  body('to')
    .trim()
    .isLength({ min: 1, max: 10 })
    .withMessage('到达站名长度必须在1-10个字符之间'),
  
  body('num')
    .isInt({ min: 1, max: 100000 })
    .withMessage('购票数量必须是1-100000之间的整数'),
  
  body('queue')
    .optional()
    .isBoolean()
    .withMessage('候补标志必须是布尔值')
];

// 添加车次验证
const validateAddTrain = [
  body('trainId')
    .trim()
    .isLength({ min: 1, max: 20 })
    .withMessage('车次ID长度必须在1-20个字符之间'),
  
  body('stationNum')
    .isInt({ min: 2, max: 100 })
    .withMessage('车站数量必须是2-100之间的整数'),
  
  body('seatNum')
    .isInt({ min: 1, max: 100000 })
    .withMessage('座位数量必须是1-100000之间的整数'),
  
  body('stations')
    .isArray({ min: 2, max: 100 })
    .withMessage('车站列表必须包含2-100个车站'),
  
  body('stations.*')
    .trim()
    .isLength({ min: 1, max: 10 })
    .withMessage('车站名长度必须在1-10个字符之间'),
  
  body('prices')
    .isArray()
    .withMessage('价格列表必须是数组'),
  
  body('prices.*')
    .isInt({ min: 0, max: 100000 })
    .withMessage('价格必须是0-100000之间的整数'),
  
  body('startTime')
    .matches(/^\d{2}:\d{2}$/)
    .withMessage('发车时间格式必须为HH:MM'),
  
  body('type')
    .isIn(['G', 'D', 'T', 'K'])
    .withMessage('车次类型必须是G、D、T或K')
];

// 检查验证结果
function checkValidationResult(req, res, next) {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    const errorMessages = errors.array().map(error => error.msg);
    
    if (req.xhr || req.headers.accept.indexOf('json') > -1) {
      return res.status(400).json({
        success: false,
        message: errorMessages.join(', '),
        errors: errors.array()
      });
    } else {
      // 对于表单提交，重新渲染页面并显示错误
      const viewName = req.path.slice(1) || 'index';
      return res.status(400).render(viewName, {
        title: getPageTitle(viewName),
        errors: errorMessages,
        formData: req.body,
        user: req.session?.user || null
      });
    }
  }
  next();
}

// 获取页面标题
function getPageTitle(viewName) {
  const titles = {
    'login': '用户登录',
    'register': '用户注册',
    'search': '车票查询',
    'orders': '我的订单',
    'profile': '个人资料',
    'admin': '系统管理'
  };
  return titles[viewName] || '火车票管理系统';
}

module.exports = {
  validateLogin,
  validateRegister,
  validateTicketSearch,
  validateBuyTicket,
  validateAddTrain,
  checkValidationResult
};
